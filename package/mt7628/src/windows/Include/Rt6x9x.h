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
    RT6X9X.h

    Abstract: 
    RT6X9X features (RT6590/RT6592/MT7601)

    Revision History:
    Who                         When            What
    -------------------     ----------      ----------------------------------------------
    Ian Tang and Brenton Wu     2011/11/23      Start implementation
    Ian Tang                    2012/3/19       Add MT7650 (RT6590)
    BJ Chiang                   2014/4/10       Remove MT7650 (RT6590)        
*/
#ifndef __RT6X9X_H__
#define __RT6X9X_H__

#pragma pack(push, struct_pack1)
#pragma pack(1)


#define NINE_DWORD (9)

extern PMP_ADAPTER    G_pAdapter;

//
// VHT NICs
//
#define VHT_NIC(_pAd)               (0)

//
// VHT capability
//
#define VHT_CAPABLE(_pPort)           (_pPort->CommonCfg.DesiredVhtPhy.bVhtEnable == TRUE)

//
// RF read/write protection between Andes firmware and Wi-Fi driver
//
#define RF_READ_WRITE_PROTECTION_CAPABLE(_pAd) (0)

//
// 1x1 VHT peer STA
//
//2 TODO: Based on the VHT IEs
//
#define VHT_NSS0_PEER_STA       (FALSE)

//
// 2x2 VHT peer STA
//
//2 TODO: Based on the VHT IEs
//
#define VHT_NSS1_PEER_STA       (FALSE)

//
// NIC using Andes firmware
//
#define ANDES_FW_NIC(_pAd)      (0)

//
// Andes firmware capability
//
#define ANDES_FW_CAPABLE(_pAd) (_pAd->HwCfg.FwCtrl.bAndesFwCapable == TRUE)

//
// MISC_CTRL
//
#define MISC_CTRL        0x0064
#ifdef BIG_ENDIAN
typedef union   _MISC_CTRL_STRUC    {
    struct  {
        ULONG   Reserved:2;
        ULONG   PERST_N:1;
        ULONG   XTAL_SEL_20:1;
        ULONG   EXT_EE_SEL:1;
        ULONG   CHIP_MODE:3;
        ULONG   Unknown1:14;    // Unknown, wait for ASIC provide
        ULONG   eFuseLoc:1;    // 1: Read WLAN scope of eFuse,   0: Read BT scope of eFuse
        ULONG   EEPROMLoc:1;   // 1: Read WLAN scope of EEPROM,  0: Read BT scope of EEPROM
        ULONG   Unknown2:5;     // Unknown, wait for ASIC provide
        ULONG   CSR_WAKEUP_OPT:2;
        ULONG   CSR_WAKEUP_N:1;
    }   field;
    ULONG           word;
}   MISC_CTRL_STRUC, *PMISC_CTRL_STRUC;
#else
typedef union   _MISC_CTRL_STRUC    {
    struct  {
        ULONG   CSR_WAKEUP_N:1;
        ULONG   CSR_WAKEUP_OPT:2;
        ULONG   Unknown2:5;     // Unknown, wait for ASIC provide
        ULONG   EEPROMLoc:1;   // 1: Read WLAN scope of EEPROM,  0: Read BT scope of EEPROM
        ULONG   eFuseLoc:1;    // 1: Read WLAN scope of eFuse,   0: Read BT scope of eFuse      
        ULONG   Unknown1:14;    // Unknown, wait for ASIC provide
        ULONG   CHIP_MODE:3;
        ULONG   EXT_EE_SEL:1;
        ULONG   XTAL_SEL_20:1;
        ULONG   PERST_N:1;      
        ULONG   Reserved:2;     
    }   field;
    ULONG           word;
}   MISC_CTRL_STRUC, *PMISC_CTRL_STRUC;
#endif

//
// The VHT control of peer STA (BSS entry)
//
typedef struct _VHT_PEER_STA_CTRL_BSS_ENTRY
{
    BOOLEAN bVhtCapable;
    VHT_MIMO_CTRL_FIELD     VhtMimoCtrlField; // VHT MIMO control field
    VHT_CAP_IE          VhtCapabilityIE; // VHT capabilities IE
    VHT_OP_IE           VhtOperationIE; // VHT operation IE
    VHT_EXT_BSS_LOAD_IE         VhtExtBssLoadIE; // VHT BSS load IE
} VHT_PEER_STA_CTRL_BSS_ENTRY, *PVHT_PEER_STA_CTRL_BSS_ENTRY;

//
// The VHT control of peer STA (MAC table)
//
typedef struct _VHT_PEER_STA_CTRL
{
    BOOLEAN              bVhtCapable; // VHT STA
    VHT_MIMO_CTRL_FIELD  VhtMimoCtrlField; // VHT MIMO control field
    VHT_CAP_IE           VhtCapability; // VHT capabilities IE
    VHT_OP_IE            VhtOperation; // VHT operation IE
    VHT_EXT_BSS_LOAD_IE  VhtExtBssLoadIE; // VHT BSS load IE
    UCHAR MaxBW; // Max bandwidth the STA can transmit
    UCHAR MaxNss; // Max Nss the STA can transmit
    BOOLEAN bOperatingModeNotificationCapable; // It's capable of Operating Mode Notification
} VHT_PEER_STA_CTRL, *PVHT_PEER_STA_CTRL;

//
// VHT control (NIC itself)
//
typedef struct _VHT_CTRL
{
    VHT_MIMO_CTRL_FIELD             VhtMimoCtrlField; // VHT MIMO control field
    BOOLEAN             bVhtCapable;
    struct
    {       
        //BOOLEAN              bVhtCapable;
        //VHT_MIMO_CTRL_FIELD             VhtMimoCtrlField; // VHT MIMO control field
        VHT_CAP_IE                      VhtCapIE; // VHT capabilities IE, ID=191
        VHT_OP_IE                       VhtOpIE; // VHT operation IE, ID=192
        VHT_EXT_BSS_LOAD_IE             VhtExtBssLoadIE; // VHT BSS load IE, ID=193
        VHT_WIDE_BW_CHANNEL_SWITCH_IE   VhtWideBwChannelSwitchIE; // VHT Wide BW Channel switch IE, ID=194
        VHT_TX_POWER_ENVELOPE_IE        VhtTxPowerEnvelopeIE;// VHT Tx Power Envelope IE, ID=195
        VHT_EXT_POWER_CONSTRAINT_IE     VhtExtPowerConstraintIE;// VHT Ext Power Constraint IE, ID=196
        VHT_AID_IE                      VhtAidIE;// VHT AID IE, ID=197
        VHT_QUIET_CHANNEL_IE            VhtQuietChannelIE;// VHT Quiet Channel IE, ID=198
    } StaVhtIEs;

    struct
    {       
        //BOOLEAN              bVhtCapable;
        //VHT_MIMO_CTRL_FIELD             VhtMimoCtrlField; // VHT MIMO control field
        VHT_CAP_IE                      VhtCapIE; // VHT capabilities IE, ID=191
        VHT_OP_IE                       VhtOpIE; // VHT operation IE, ID=192
        VHT_EXT_BSS_LOAD_IE             VhtExtBssLoadIE; // VHT BSS load IE, ID=193
        VHT_WIDE_BW_CHANNEL_SWITCH_IE   VhtWideBwChannelSwitchIE; // VHT Wide BW Channel switch IE, ID=194
        VHT_TX_POWER_ENVELOPE_IE        VhtTxPowerEnvelopeIE;// VHT Tx Power Envelope IE, ID=195
        VHT_EXT_POWER_CONSTRAINT_IE     VhtExtPowerConstraintIE;// VHT Ext Power Constraint IE, ID=196
        VHT_AID_IE                      VhtAidIE;// VHT AID IE, ID=197
        VHT_QUIET_CHANNEL_IE            VhtQuietChannelIE;// VHT Quiet Channel IE, ID=198
    } SoftAPVhtIEs;

} VHT_CTRL, *PVHT_CTRL;

//
// The information of bandwidth and primary 20 location
//
typedef struct _BW_PRI20_INFO
{
    UCHAR Bandwidth; // BW_XXX
    UCHAR Primary20Location; // EXTCHA_XX or PRI_CH_XXX
} BW_PRI20_INFO, *PBW_PRI20_INFO;

//
// Firmware control
//
typedef struct _FW_CTRL
{
    BOOLEAN bAndesFwCapable;

    //
    // Packet command sequence number: 
    //  0: Do not need response event (NO_PKT_CMD_RSP_EVENT)
    //  1~15: A sequence number for each packet command and need a response event
    //
    UCHAR PktCmdSeq;
} FW_CTRL, *PFW_CTRL;


//
// MAC extension (from RT6X9X)
//
// Note: 
//  1. New TXWI/RXWI structures
//  2. New order of RXD/RX_FCE_INFO/RXINFO/RXWI
//  3. New register address mapping
//
#define EXT_MAC_CAPABLE(_pAd)   (_pAd->HwCfg.HwArchitectureVersion.MacVersion == MacArchitectureVersion_MacRegRemappingWithAndesEnabled)

//
// BB extension (from RT6X9X)
//
// Note: 
//  1. Four bytes BB register read/write
//  2. Refactor BB registers
//
#define EXT_BB_CAPABLE(_pAd)    (_pAd->HwCfg.HwArchitectureVersion.BbVersion == BbArchitectureVersion_FourBytesReadWriteEnabled)

//
// EEPROM extension
//
#define EXT_EEPROM_PCIE_USB_CONCURRENT(_pAd)  (0)

//
// TXWI size
//
#define TXWI_SIZE_20_BYTES  20
#define TXWI_SIZE_16_BYTES  16

//
//
//
#define RX_DMA_LENGTH       4

//
// RXINFO
//
// Note: 
//  1. Do not access the field directly
//  2. Use READ_RXINFO_XXX and WRITE_RXINFO_XXX macros to get and set fields of RXINFO
//
typedef struct _RXINFO_STRUC
{
    ULONG   Ba:1; // BA session
    ULONG   Data:1; // Data frame
    ULONG   Null:1; // Null data frame
    ULONG   Frag:1; // Fragmented data frame (TKIP MIC will be skipped on fragmented data frame)
    ULONG   U2M:1; // Unicast to me frame
    ULONG   MC:1; // Multicast frame
    ULONG   BC:1; // Broadcast frame
    ULONG   MyBss:1; // My BSSID frame
    
    ULONG   CrcErr:1; // CRC error
    ULONG   CipherErr:2; // Cipher error - 0: successful decryption, 1:ICV error, 2:MIC error, 3:ICV error + MIC error
    ULONG   AMSDU:1; // AMSDU aggregated frame
    ULONG   HTC:1; // Four bytes HTC are padded after MAC header.
    ULONG   RSSI:1; // RSSI/SNR/PHY rate are valid.
    ULONG   L2Pad:1; // Two bytes zero are padded after MAC header.
    ULONG   AMPDU:1; // AMPDU aggregated frame
    
    ULONG   Decrypted:1; // Decrypted frame
    ULONG   BssIdx3:1; // BSS index bit 3 (use together with BSS index bit2:bit0 in RXWI)
    ULONG   WapiKeyID:1; // WAPI key ID
    ULONG   PnLength:3; // IV/EIV/PN padding length (unit: double words)
    ULONG   Reserved:6; // Reserved
    ULONG   TcpUdpChecksumBypass:1; // TCP/UDP checksum bypass (HW doesn't do checksum)
    ULONG   IpChecksumBypass:1; // IP checksum bypass (HW doesn't do checksum)
    ULONG   TcpChecksumError:1; // TCP checksum error
    ULONG   IpChecksumError:1; // IP checksum error
} RXINFO_STRUC, *PRXINFO_STRUC;

//
// READ_RXINFO_XXX and WRITE_RXINFO_XXX macros are used to get and set fields of RXINFO
//
#define READ_RXINFO_BA(_pAd, _pRxInfo) \
        ((_pRxInfo)->Ba)

#define READ_RXINFO_DATA(_pAd, _pRxInfo) \
        ((_pRxInfo)->Data)

#define READ_RXINFO_NULL_DATA(_pAd, _pRxInfo) \
        ((_pRxInfo)->Null)

#define READ_RXINFO_FRAG(_pAd, _pRxInfo) \
        ((_pRxInfo)->Frag)

#define READ_RXINFO_U2M(_pAd, _pRxInfo) \
        ((_pRxInfo)->U2M)

#define READ_RXINFO_MCAST(_pAd, _pRxInfo) \
        ((_pRxInfo)->MC)

#define READ_RXINFO_BCAST(_pAd, _pRxInfo) \
        ((_pRxInfo)->BC)

#define READ_RXINFO_MY_BSS(_pAd, _pRxInfo) \
        ((_pRxInfo)->MyBss)

#define READ_RXINFO_CRC_ERROR(_pAd, _pRxInfo) \
        ((_pRxInfo)->CrcErr)

#define READ_RXINFO_CIPHER_ERROR(_pAd, _pRxInfo) \
        ((_pRxInfo)->CipherErr)

#define READ_RXINFO_AMSDU(_pAd, _pRxInfo) \
        ((_pRxInfo)->AMSDU)

#define READ_RXINFO_HTC(_pAd, _pRxInfo) \
        ((_pRxInfo)->HTC)

#define READ_RXINFO_RSSI(_pAd, _pRxInfo) \
        ((_pRxInfo)->RSSI)

#define READ_RXINFO_L2PAD(_pAd, _pRxInfo) \
        ((_pRxInfo)->L2Pad)

#define READ_RXINFO_AMPDU(_pAd, _pRxInfo) \
        ((_pRxInfo)->AMPDU)

#define READ_RXINFO_DECRYPTED(_pAd, _pRxInfo) \
        ((_pRxInfo)->Decrypted)

#define READ_RXINFO_BSS_IDX3(_pAd, _pRxInfo) \
        ((_pRxInfo)->BssIdx3)

#define READ_RXINFO_WAPI_KEY_ID(_pAd, _pRxInfo) \
        ((_pRxInfo)->WapiKeyID)

#define READ_RXINFO_PN_LENGTH(_pAd, _pRxInfo) \
        ((_pRxInfo)->PnLength)

#define READ_RXINFO_TCP_UDP_CHECKSUM_BYPASS(_pAd, _pRxInfo) \
        ((_pRxInfo)->TcpUdpChecksumBypass)

#define READ_RXINFO_IP_CHECKSUM_BYPASS(_pAd, _pRxInfo) \
        ((_pRxInfo)->IpChecksumBypass)

#define READ_RXINFO_TcpChecksumError(_pAd, _pRxInfo) \
        ((_pRxInfo)->TcpChecksumError)

#define READ_RXINFO_IpChecksumError(_pAd, _pRxInfo) \
        ((_pRxInfo)->IpChecksumError)

#define WRITE_RXINFO_BA(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->Ba) = (_value); \
        }

#define WRITE_RXINFO_DATA(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->Data) = (_value); \
        }

#define WRITE_RXINFO_NULL_DATA(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->Null) = (_value); \
        }

#define WRITE_RXINFO_FRAG(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->Frag) = (_value); \
        }

#define WRITE_RXINFO_U2M(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->U2M) = (_value); \
        }

#define WRITE_RXINFO_MCAST(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->MC) = (_value); \
        }

#define WRITE_RXINFO_BCAST(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->BC) = (_value); \
        }

#define WRITE_RXINFO_MY_BSS(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->MyBss) = (_value); \
        }

#define WRITE_RXINFO_CRC(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->CrcErr) = (_value); \
        }

#define WRITE_RXINFO_CIPHER_ERROR(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->CipherErr) = (_value); \
        }

#define WRITE_RXINFO_AMSDU(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->AMSDU) = (_value); \
        }

#define WRITE_RXINFO_HTC(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->HTC) = (_value); \
        }

#define WRITE_RXINFO_RSSI(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->RSSI) = (_value); \
        }

#define WRITE_RXINFO_L2PAD(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->L2Pad) = (_value); \
        }

#define WRITE_RXINFO_AMPDU(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->AMPDU) = (_value); \
        }

#define WRITE_RXINFO_DECRYPTED(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->Decrypted) = (_value); \
        }

#define WRITE_RXINFO_BSS_IDX3(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->BssIdx3) = (_value); \
        }

#define WRITE_RXINFO_WAPI_KEY_ID(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->WapiKeyID) = (_value); \
        }

#define WRITE_RXINFO_PN_LENGTH(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->PnLength) = (_value); \
        }

#define WRITE_RXINFO_TCP_UDP_CHECKSUM_BYPASS(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->TcpUdpChecksumBypass) = (_value); \
        }

#define WRITE_RXINFO_IP_CHECKSUM_BYPASS(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->IpChecksumBypass) = (_value); \
        }

#define WRITE_RXINFO_TCP_CHECKSUM_ERROR(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->TcpChecksumError) = (_value); \
        }

#define WRITE_RXINFO_IP_CHECKSUM_ERROR(_pAd, _pRxInfo, _value) \
        { \
            ((_pRxInfo)->IpChecksumError) = (_value); \
        }

//
// RXWI size
//
#define RXWI_SIZE_28_BYTES  28
#define RXWI_SIZE_24_BYTES  24
#define RXWI_SIZE_20_BYTES  20
#define RXWI_SIZE_16_BYTES  16

//
// Nss
//
#define NSS_0       (0) // 1x1
#define NSS_1       (1) // 2x2
#define NSS_2       (2) // 3x3
#define NSS_ALL     (0xFF) // All Nss

//
// MCS/NSS in HT mode (for TXWI and RXWI)
//
// Note: 
//  MCS[6:3] = NSS (0~1)
//  MCS[2:0] = MCS (0~7)
//
#define HT_MCS_0            (0x00)
#define HT_MCS_1            (0x01)
#define HT_MCS_2            (0x02)
#define HT_MCS_3            (0x03)
#define HT_MCS_4            (0x04)
#define HT_MCS_5            (0x05)
#define HT_MCS_6            (0x06)
#define HT_MCS_7            (0x07)
#define HT_MCS_8            (0x08)
#define HT_MCS_9            (0x09)
#define HT_MCS_10       (0x0A)
#define HT_MCS_11       (0x0B)
#define HT_MCS_12       (0x0C)
#define HT_MCS_13       (0x0D)
#define HT_MCS_14       (0x0E)
#define HT_MCS_15       (0x0F)

//
// The base address of MAC register section
//
//2 TODO: Add complete base address
//
#define SYS_CTRL_REG_BASE                       (0x0000)
#define IF_DMA_REG_BASE                         (0x0200)
#define PBF_REG_BASE                                (0x0400)
#define FCE_FCE_PDMA_REG_BASE                   (0x0800)

//
//  Status flags
//
#define fCLIENT_STATUS_WMM_CAPABLE          0x00000001 // Client can parse QoS data frame
#define fCLIENT_STATUS_AGGREGATION_CAPABLE  0x00000002 // Client can receive Ralink's proprietary TX aggregation frame
#define fCLIENT_STATUS_PIGGYBACK_CAPABLE    0x00000004 // Client support piggy-back
#define fCLIENT_STATUS_AMSDU_INUSED         0x00000008
#define fCLIENT_STATUS_SGI20_CAPABLE        0x00000010 // Short GI on BW20
#define fCLIENT_STATUS_SGI40_CAPABLE        0x00000020 // Short GI on BW40
#define fCLIENT_STATUS_SGI80_CAPABLE        0x00000040 // Short GI on BW80
#define fCLIENT_STATUS_SGI160_8080_CAPABLE  0x00000080 // Short GI on BW160 or BW80+80
#define fCLIENT_STATUS_HT_TX_STBC_CAPABLE   0x00000100 // HT Tx STBC
#define fCLIENT_STATUS_HT_RX_STBC_CAPABLE   0x00000200 // HT Rx STBC
#define fCLIENT_STATUS_VHT_TX_STBC_CAPABLE  0x00000400 // VHT Tx STBC
#define fCLIENT_STATUS_VHT_RX_STBC_CAPABLE  0x00000800 // VHT Rx STBC
#define fCLIENT_STATUS_RDG_CAPABLE          0x00001000
#define fCLIENT_STATUS_RALINK_CHIPSET       0x00002000
#if UAPSD_AP_SUPPORT
#define fCLIENT_STATUS_APSD_CAPABLE         0x00004000 // UAPSD STA
#endif

//
// VHT BW80 delta power control (+4~-4dBm) for per-rate Tx power control
//
#define EEPROM_VHT_BW80_TX_POWER_DELTA  (0x11E)

//
// LDO CTRL 1
//
#define LDO_CTRL1 (0x0070)

#define PBF_CFG(_pAd)               ((EXT_MAC_CAPABLE(_pAd)) ? (PBF_REG_BASE + 0x0004) : (0x0408))

/////////////////////////////////////////////////////////////////////////////////////////////
// PBF registers
/////////////////////////////////////////////////////////////////////////////////////////////
#define TX_MAX_PCNT 0x408
#define RX_MAX_PCNT 0x40C

#define BUFFER_RESET    0x00000010

#define KICK_NULL1_FRAME    (0x00000040)

#define BCN_OFFSET0 ((EXT_MAC_CAPABLE(G_pAdapter)) ? (PBF_REG_BASE + 0x1C) : (0x042C))
#define BCN_OFFSET1 ((EXT_MAC_CAPABLE(G_pAdapter)) ? (PBF_REG_BASE + 0x20) : (0x0430))
#define BCN_OFFSET2 (PBF_REG_BASE + 0x24)
#define BCN_OFFSET3 (PBF_REG_BASE + 0x28)


/////////////////////////////////////////////////////////////////////////////////////////////
// FCE/FCE PDMA registers
/////////////////////////////////////////////////////////////////////////////////////////////

#define FCE_CTRL                (FCE_FCE_PDMA_REG_BASE + 0x0000)

//
// Registers for Security
//
#define PAIRWISE_KEY_TABLE_BASE         ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x4000 + 0x4000) : (0x4000))       // 32-byte * 256-entry =  -byte
#define MAC_IVEIV_TABLE_BASE            ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x6000 + 0x4000) : (0x6000))      // 8-byte * 256-entry =  -byte
#define PAIRWISE_IVEIV_TABLE_BASE       ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x6000 + 0x4000) : (0x6000))      // 8-byte * 256-entry =  -byte
#define MAC_WCID_ATTRIBUTE_BASE         ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x6800 + 0x4000) : (0x6800))      // 4-byte * 256-entry =  -byte
#define SHARED_KEY_TABLE_BASE           ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x6c00 + 0x4000) : (0x6c00))      // 32-byte * 16-entry = 512-byte
#define SHARED_KEY_MODE_BASE            ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x7000 + 0x4000) : (0x7000))      // 32-byte * 16-entry = 512-byte
#define MAC_WAPI_PN_TABLE_BASE          ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x7800 + 0x4000) : (0x7800))

#define HW_DEBUG_SETTING_BASE           (ULONG)((EXT_MAC_CAPABLE(G_pAdapter)) ? (0xA7f0) : (0x77f0)) 
#define HW_DEBUG_SETTING_BASE2          ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0xA770) : (0x7770)) 

//
// on-chip BEACON frame space - base address = 0x7800
//
//2 todo: define 0x4000
#define HW_BEACON_BASE0     ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0xC000) : (0x7800))
#define HW_BEACON_BASE1     ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0xC200) : (0x7900))
#define HW_BEACON_BASE2     ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0xC400) : (0x7A00))
#define HW_BEACON_BASE3     ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0xC600) : (0x7B00))
#define HW_BEACON_BASE4     ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0xC800) : (0x7C00))
#define HW_BEACON_BASE5     ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0xCA00) : (0x7D00))
#define HW_BEACON_BASE6     ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0xCC00) : (0x7E00))
#define HW_BEACON_BASE7     ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0xCE00) : (0x7F00))

#define HW_BEACON_BUFFER_SIZE       ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x200) : (0x100))
#define HW_BEACON_OFFSET            ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x00) : (0xE0))
#define HW_BEACON_UNITS             ((EXT_MAC_CAPABLE(G_pAdapter)) ? (0x8) : (0x4))

//
// define for RT6X9X and later (0x0400), distinguish from "PBF_SYS_CTRL_STRUC"
//

#ifdef BIG_ENDIAN
typedef union   _BW_CALIBRATION_STRUC   {
    struct  {
        ULONG       reserved:8; // Don't Care       
        ULONG       Partial:8;  // 0:Full Calibration, 1:Partial
        ULONG       BW:8;   // 0:BW20, 1:BW40, 2:BW80       
        ULONG       RxTx:8; // 0:RX, 1:TX
    } field;
    ULONG           word;
}   BW_CALIBRATION_STRUC, *PBW_CALIBRATION_STRUC;
#else
typedef union   _BW_CALIBRATION_STRUC   {
    struct  {
        ULONG       RxTx:8; // 0:RX, 1:TX
        ULONG       BW:8;   // 0:BW20, 1:BW40, 2:BW80
        ULONG       Partial:8;  // 0:Full Calibration, 1:Partial
        ULONG       reserved:8; // Don't Care
    } field;
    ULONG           word;
}   BW_CALIBRATION_STRUC, *PBW_CALIBRATION_STRUC;
#endif

#ifdef BIG_ENDIAN
typedef union _PBF_SYS_CTRL_STRUC_EXT
{
    struct
    {
        ULONG   Reserved3:7; // Reserved
        ULONG   CSR_TEST_EN:1;
        ULONG   MAC_CLK_SEL:2;
        ULONG   PWRSV_EN:2; //MAC idle power saving mode enable
        ULONG   SHR_MSEL:1; // Shared memory access selection
        ULONG   PBF_MSEL:2; // Packet buffer memory access selection
        ULONG   Reserved2:5; 
        ULONG   PBF_CLK_EN:1; 
        ULONG   MAC_CLK_EN:1; 
        ULONG   Reserved1:6; 
        ULONG   PBF_RESET:1; // PBF hardware reset
        ULONG   MAC_RESET:1; // MAC hardware reset      
        ULONG   Reserved0:2;
    }   field;

    ULONG       word;
} PBF_SYS_CTRL_STRUC_EXT, *PPBF_SYS_CTRL_STRUC_EXT;
#else
typedef union _PBF_SYS_CTRL_STRUC_EXT
{
    struct
    {       
        ULONG   Reserved0:2;
        ULONG   MAC_RESET:1; // MAC hardware reset
        ULONG   PBF_RESET:1; // PBF hardware reset
        ULONG   Reserved1:6; 
        ULONG   MAC_CLK_EN:1;
        ULONG   PBF_CLK_EN:1; 
        ULONG   Reserved2:5; 
        ULONG   PBF_MSEL:2; // Packet buffer memory access selection
        ULONG   SHR_MSEL:1; // Shared memory access selection
        ULONG   PWRSV_EN:2; //MAC idle power saving mode enable
        ULONG   MAC_CLK_SEL:2;
        ULONG   CSR_TEST_EN:1;
        ULONG   Reserved3:7; // Reserved
    }   field;
    ULONG       word;
} PBF_SYS_CTRL_STRUC_EXT, *PPBF_SYS_CTRL_STRUC_EXT;
#endif

//
// define for RT6X9X and later (0x0404), distinguish from "PBF_CFG_STRUC"
//
#ifdef BIG_ENDIAN
typedef union _PBF_CFG_STRUC_EXT
{
    struct  {
        ULONG   :8;
        ULONG   Tx1QNumber:3;
        ULONG   Tx2QNumber:5;
        ULONG   HCCANull0AutoMode:1;
        ULONG   HCCANull1AutoMode:1;
        ULONG   RxDropMode:1;
        ULONG   Tx0QMode:1;
        ULONG   Tx1QMode:1;
        ULONG   Tx2QMode:1;
        ULONG   Rx0QMode:1;
        ULONG   HCCAMode:1;
        ULONG   Rsvd:3;
        ULONG   Tx0QEnable:1;
        ULONG   Tx1QEnable:1;
        ULONG   Tx2QEnable:1;
        ULONG   Rx0QEnable:1;
        ULONG   Rsvd1:1;
    } Default;

    struct
    {
        ULONG   Reserved:23;
        ULONG   RxDropMode:1;
        ULONG   Null2Sel:3;
        ULONG   Rx0QEnable:1;
        ULONG   Tx3QEnable:1;
        ULONG   Tx2QEnable:1;
        ULONG   Tx1QEnable:1;
        ULONG   Tx0QEnable:1;
    } Ext;

    ULONG word;
}   PBF_CFG_STRUC_EXT, *PPBF_CFG_STRUC_EXT;
#else
typedef union _PBF_CFG_STRUC_EXT
{
    struct  {
        ULONG   Rsvd1:1;
        ULONG   Rx0QEnable:1;
        ULONG   Tx2QEnable:1;
        ULONG   Tx1QEnable:1;
        ULONG   Tx0QEnable:1;
        ULONG   Rsvd:3;
        ULONG   HCCAMode:1;
        ULONG   Rx0QMode:1;
        ULONG   Tx2QMode:1;
        ULONG   Tx1QMode:1;
        ULONG   Tx0QMode:1;
        ULONG   RxDropMode:1;
        ULONG   HCCANull1AutoMode:1;
        ULONG   HCCANull0AutoMode:1;
        ULONG   Tx2QNumber:5;
        ULONG   Tx1QNumber:3;
        ULONG   :8;     
    } Default;

    struct
    {
        ULONG   Tx0QEnable:1;
        ULONG   Tx1QEnable:1;
        ULONG   Tx2QEnable:1;
        ULONG   Tx3QEnable:1;
        ULONG   Rx0QEnable:1;
        ULONG   Null2Sel:3;
        ULONG   RxDropMode:1;
        ULONG   Reserved:23;
    } Ext;
    
    ULONG word;
} PBF_CFG_STRUC_EXT, *PPBF_CFG_STRUC_EXT;
#endif

#define READ_PBF_CFG_TX0QENABLE(_pAd, _pPbfCfg) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxWI)->Ext.Tx0QEnable) \
        : ((_pTxWI)->Default.Tx0QEnable))

#define READ_PBF_CFG_TX1QENABLE(_pAd, _pPbfCfg) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxWI)->Ext.Tx1QEnable) \
        : ((_pTxWI)->Default.Tx0QEnable))

#define READ_PBF_CFG_TX2QENABLE(_pAd, _pPbfCfg) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxWI)->Ext.Tx2QEnable) \
        : ((_pTxWI)->Default.Tx0QEnable))

#define READ_PBF_CFG_TX3QENABLE(_pAd, _pPbfCfg) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxWI)->Ext.Tx0QEnable) \
        : (0))

#define WRITE_PBF_CFG_TX0QENABLE(_pAd, _pPbfCfg, _value) \
        { \
            if (EXT_MAC_CAPABLE(_pAd)) \
            { \
                ((_pPbfCfg)->Ext.Tx0QEnable) = (_value); \
            } \
            else \
            { \
                ((_pPbfCfg)->Default.Tx0QEnable) = (_value); \
            } \
        }

#define WRITE_PBF_CFG_TX1QENABLE(_pAd, _pPbfCfg, _value) \
        { \
            if (EXT_MAC_CAPABLE(_pAd)) \
            { \
                ((_pPbfCfg)->Ext.Tx1QEnable) = (_value); \
            } \
            else \
            { \
                ((_pPbfCfg)->Default.Tx1QEnable) = (_value); \
            } \
        }

#define WRITE_PBF_CFG_TX2QENABLE(_pAd, _pPbfCfg, _value) \
        { \
            if (EXT_MAC_CAPABLE(_pAd)) \
            { \
                ((_pPbfCfg)->Ext.Tx2QEnable) = (_value); \
            } \
            else \
            { \
                ((_pPbfCfg)->Default.Tx2QEnable) = (_value); \
            } \
        }

#define WRITE_PBF_CFG_TX3QENABLE(_pAd, _pPbfCfg, _value) \
        { \
            if (EXT_MAC_CAPABLE(_pAd)) \
            { \
                ((_pPbfCfg)->Ext.Tx3QEnable) = (_value); \
            } \
        }

#define WRITE_PBF_CFG_RX0QENABLE(_pAd, _pPbfCfg, _value) \
        { \
            if (EXT_MAC_CAPABLE(_pAd)) \
            { \
                ((_pPbfCfg)->Ext.Rx0QEnable) = (_value); \
            } \
            else \
            { \
                ((_pPbfCfg)->Default.Rx0QEnable) = (_value); \
            } \
        }

//
// define for RT6X9X and later (0x0410)
//
#ifdef BIG_ENDIAN
typedef union _PBUF_CTRL_STRUC
{
    struct
    {
        ULONG   Reserved2:20;
        
        ULONG   WRITE_TX0Q:1;
        ULONG   WRITE_TX1Q:1;
        ULONG   WRITE_TX2Q:1;
        ULONG   WRITE_RX0Q:1;
        
        ULONG   NULL0_KICK:1;
        ULONG   NULL1_KICK:1;
        ULONG   BUF_RESET:1;
        ULONG   NULL2_KICK:1;
        
        ULONG   READ_TX0Q:1;
        ULONG   READ_TX1Q:1;
        ULONG   READ_TX2Q:1;
        ULONG   READ_RX0Q:1;
    } Default;
    
    struct
    {
        ULONG   Reserved2:27;
        ULONG   BUF_RESET:1;
        ULONG   Reserved1:1;
        ULONG   NULL2_KICK:1;
        ULONG   NULL1_KICK:1;
        ULONG   NULL0_KICK:1;
    } Ext;

    ULONG word;
}   PBUF_CTRL_STRUC, *PBUF_CTRL_STRUC;
#else
typedef union _PBUF_CTRL_STRUC
{
    struct
    {
        ULONG   READ_RX0Q:1;
        ULONG   READ_TX2Q:1;
        ULONG   READ_TX1Q:1;
        ULONG   READ_TX0Q:1;
        
        ULONG   NULL2_KICK:1;
        ULONG   BUF_RESET:1;
        ULONG   NULL1_KICK:1;
        ULONG   NULL0_KICK:1;
        
        ULONG   WRITE_RX0Q:1;
        ULONG   WRITE_TX2Q:1;
        ULONG   WRITE_TX1Q:1;
        ULONG   WRITE_TX0Q:1;
        
        ULONG   Reserved2:20;
    } Default;
    struct
    {
        ULONG   NULL0_KICK:1;
        ULONG   NULL1_KICK:1;
        ULONG   NULL2_KICK:1;
        ULONG   Reserved1:1;
        ULONG   BUF_RESET:1;        
        ULONG   Reserved:27;
    } Ext;  
    ULONG word;
} BUF_CTRL_STRUC, *PBUF_CTRL_STRUC;
#endif

#ifdef BIG_ENDIAN
typedef union   _INT_SOURCE_CSR_STRUC   {
    struct  {
        ULONG           :14;
        ULONG           TxCoherent:1;
        ULONG           RxCoherent:1;
        ULONG           GPTimer:1;
        ULONG           AutoWakeup:1;//bit14
        ULONG           TXFifoStatusInt:1;//FIFO Statistics is full, sw should read 0x171c
        ULONG           PreTBTT:1;
        ULONG           TBTTInt:1;
        ULONG           RxTxCoherent:1;
        ULONG           MCUCommandINT:1;
        ULONG           MgmtDmaDone:1;
        ULONG           HccaDmaDone:1;
        ULONG           Ac3DmaDone:1;
        ULONG           Ac2DmaDone:1;
        ULONG           Ac1DmaDone:1;
        ULONG       Ac0DmaDone:1;
        ULONG       RxDone:1;
        ULONG       TxDelayINT:1;   //delayed interrupt, not interrupt until several int or time limit hit
        ULONG       RxDelayINT:1; //dealyed interrupt
    }   Default;
    struct  {
        ULONG           Reserved_4:4;
        ULONG           TxDelayINT:1;
        ULONG           RxDelayINT:1;
        ULONG           Reserved_3:1;
        ULONG           GPTimer:1;
        ULONG           AutoWakeup:1;
        ULONG           TXFifoStatusInt:1;//FIFO Statistics is full
        ULONG           PreTBTT:1;
        ULONG           TBTTInt:1;
        ULONG           MCUCommandINT:1;
        ULONG           RxTxCoherent:1;
        ULONG           TxCoherent:1;
        ULONG           RxCoherent:1;
        ULONG           Reserved_2:2;
        ULONG           Ac9DmaDone:1; // TX Queue 9
        ULONG           Ac8DmaDone:1; // TX Queue 8
        ULONG           Ac7DmaDone:1; // TX Queue 7
        ULONG           Ac6DmaDone:1; // TX Queue 6
        ULONG           MgmtDmaDone:1; // TX Queue 5 , MgmtDmaDone?
        ULONG           HccaDmaDone:1; // TX Queue 4 , HccaDmaDone?
        ULONG           Ac3DmaDone:1; // TX Queue 3
        ULONG           Ac2DmaDone:1; // TX Queue 2
        ULONG           Ac1DmaDone:1; // TX Queue 1
        ULONG           Ac0DmaDone:1; // TX Queue 0
        ULONG           Reserved_1:2;
        ULONG           RxDone_1:1;
        ULONG           RxDone_0:1;
    }   Ext;    
    ULONG           word;
}   INT_SOURCE_CSR_STRUC, *PINT_SOURCE_CSR_STRUC;
#else
typedef union   _INT_SOURCE_CSR_STRUC   {
    struct  {
        ULONG       RxDelayINT:1;
        ULONG       TxDelayINT:1;
        ULONG       RxDone:1;
        ULONG       Ac0DmaDone:1;//4      
        ULONG           Ac1DmaDone:1;
        ULONG           Ac2DmaDone:1;
        ULONG           Ac3DmaDone:1;
        ULONG           HccaDmaDone:1; // bit7
        ULONG           MgmtDmaDone:1;
        ULONG           MCUCommandINT:1;//bit 9
        ULONG           RxTxCoherent:1;
        ULONG           TBTTInt:1;
        ULONG           PreTBTT:1;
        ULONG           TXFifoStatusInt:1;//FIFO Statistics is full, sw should read 0x171c
        ULONG           AutoWakeup:1;//bit14
        ULONG           GPTimer:1;
        ULONG           RxCoherent:1;//bit16
        ULONG           TxCoherent:1;
        ULONG           :14;
    }   Default;
    struct  {
        ULONG           RxDone_0:1;
        ULONG           RxDone_1:1;
        ULONG           Reserved_1:2;
        ULONG           Ac0DmaDone:1; // TX Queue 0
        ULONG           Ac1DmaDone:1; // TX Queue 1
        ULONG           Ac2DmaDone:1; // TX Queue 2
        ULONG           Ac3DmaDone:1; // TX Queue 3
        ULONG           HccaDmaDone:1; // TX Queue 4 , HccaDmaDone?
        ULONG           MgmtDmaDone:1; // TX Queue 5 , MgmtDmaDone?
        ULONG           Ac6DmaDone:1; // TX Queue 6
        ULONG           Ac7DmaDone:1; // TX Queue 7
        ULONG           Ac8DmaDone:1; // TX Queue 8
        ULONG           Ac9DmaDone:1; // TX Queue 9
        ULONG           Reserved_2:2;
        ULONG           RxCoherent:1;
        ULONG           TxCoherent:1;
        ULONG           RxTxCoherent:1;
        ULONG           MCUCommandINT:1;    
        ULONG           TBTTInt:1;
        ULONG           PreTBTT:1;
        ULONG           TXFifoStatusInt:1;//FIFO Statistics is full
        ULONG           AutoWakeup:1;
        ULONG           GPTimer:1;
        ULONG           Reserved_3:1;
        ULONG           RxDelayINT:1;
        ULONG           TxDelayINT:1;
        ULONG           Reserved_4:4;
    }   Ext;
    ULONG           word;
} INT_SOURCE_CSR_STRUC, *PINT_SOURCE_CSR_STRUC;
#endif


typedef union   _TXINFO_STRUC   {
    struct  {
        ULONG       USBDMATxPktLen:16;  //used ONLY in USB bulk Aggregation,  Total byte counts of all sub-frame.   
        ULONG       SwUseSegIdx:6;
        ULONG       SwUsepad:2;
        ULONG       WIV:1;  // Wireless Info Valid. 1 if Driver already fill WI,  o if DMA needs to copy WI to correctposition
        ULONG       QSEL:2; // select on-chip FIFO ID for 2nd-stage output scheduler.0:MGMT, 1:HCCA 2:EDCA
        ULONG       SwUseSegmentEnd:1; // Software use to indicate this is last packet, Next packet need to be rounded.
        ULONG       SwUseAMSDU:1; // Software use only.  This RA is under AMSUD policy. So make AMSDU at BulkOutData
        ULONG       SwUseNonQoS:1;  // Software use only.  Indicate this is DHCP or EAPOL frame
        ULONG       USBDMANextVLD:1;    //used ONLY in USB bulk Aggregation, NextValid  
        ULONG       USBDMATxburst:1;//used ONLY in USB bulk Aggre. Force USB DMA transmit frame from current selected endpoint
    }   Default;
    
    struct  {
        ULONG       USBDMATxPktLen:14;  //used ONLY in USB bulk Aggregation,  Total byte counts of all sub-frame.
        ULONG       USBDMATxPktRsv:2;
        ULONG       USBDMANextVLD:1;
        ULONG       USBDMATxburst:1;
        //ULONG     Reserved1:1;
        ULONG       SwUseSegmentEnd:1;
        ULONG       Is80211:1;  
        ULONG       TSO:1;
        ULONG       CSO:1; 
        //ULONG     Reserved2:2;
        ULONG       SwUseAMSDU:1; // Software use only.  This RA is under AMSUD policy. So make AMSDU at BulkOutData
        ULONG       SwUseNonQoS:1;  // Software use only.  Indicate this is DHCP or EAPOL frame
        ULONG       WIV:1; 
        ULONG       QSEL:2;
        ULONG       DPort:3;
        ULONG       InfoType:2;
    }   Ext;

    struct // Packet command
    {
        ULONG   PktCmdParametersLength:14; // Length of parameters
        ULONG   PktCmdParametersRsv:2;
        ULONG   PktCmdSeq:4; // Packet command sequence (NO_PKT_CMD_RSP_EVENT or sequence)
        ULONG   PktCmdType:7; // Packet command type (PKT_CMD_TYPE_XXX)
        ULONG   DPort:3; // Destination port, which the FCE port (PORT_TYPE_XXX)

        //
        // Information type (INFO_TYPE_XXX)
        //
        // Note that this field is at the same location of PktCmd.InfoType
        //
        ULONG   InfoType:2;
    } PktCmd; // Packet command
    
    ULONG           word;
} TXINFO_STRUC, *PTXINFO_STRUC;

typedef struct  _HARD_TRANSMIT_INFO
{
    UCHAR                           QueIdx;
    UCHAR                           PortNumber;
    UCHAR                           TxRate;
    UCHAR                           MpduRequired;
    UCHAR                           UserPriority;
    UCHAR                           FrameGap;
    ULONG                           TotalPacketLength;
    ULONG                           LengthQosPAD;
    ULONG                           This80211HdrLen;
    USHORT                          Protocol;
    BOOLEAN                         bEAPOLFrame;
    DOT11_CIPHER_ALGORITHM          Cipher;
    USHORT                          TransferBufferLength;
    USHORT                          AckDuration;
    USHORT                          EncryptionOverhead;
    UCHAR                           CipherAlg;
    BOOLEAN                         bAckRequired;
    BOOLEAN                         bInsertTimestamp;
    BOOLEAN                         bNDPAnnouncement;
    BOOLEAN                         bSkipTxBF;
    UCHAR                           MimoPs;
    UCHAR                           KeyIdx;
    BOOLEAN                         bMcast;
    BOOLEAN                         bTXBA;
    UCHAR                           PID;
    UCHAR                           BAWinSize;
    BOOLEAN                         bHTC;
    BOOLEAN                         bWMM;
    BOOLEAN                             bHtVhtFrame;
    UCHAR                           Wcid;
    UCHAR                           RABAOriIdx;
    BOOLEAN                         bPiggyBack;
    BOOLEAN                         bAMSDU;
    BOOLEAN                         bGroupMsg2;
    BOOLEAN                         bDHCPv6;
    BOOLEAN                         bDHCPFrame;
    UCHAR                           NextValid;
    BOOLEAN                         bDLSFrame;
    BOOLEAN                         bTDLSFrame;
    USHORT                          ExemptionActionType;
    UCHAR                           MoreData; // frame control field has pHeader80211->FC.MoreFrag = 1
    BOOLEAN                         bTxBurst;
    // WAPI
    BOOLEAN                         bWAIFrame;
    BOOLEAN                         bIsUnicastFrame;
    BOOLEAN                         bIsLastFrag;
    BOOLEAN                         bIsFrag;
    // 11w
    BOOLEAN                         bWiv;
    // EAPOL frame will use FIFO_MGMT
    UCHAR                           TxMICValue[8];
    UCHAR                           OriginalQueIdx;
    // AdhocN Support
    BOOLEAN                         bAdhocN_ChangeMulticastRate;
    UCHAR                           PortSecured;
    
    UCHAR                           SwUseSegIdx;

    UCHAR                           FragmentByteForTxD;
    BOOLEAN                         bBARPacket;
    ULONG                           BARStartSequence;
} HARD_TRANSMIT_INFO, *PHARD_TRANSMIT_INFO;



#define WRITE_TXINFO_USBDMATxPktLen(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.USBDMATxPktLen) = (_value); \
    } \
    else \
    { \
        ((_pTxInfo)->Default.USBDMATxPktLen) = (_value); \
    } \
}


#define WRITE_TXINFO_WIV(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.WIV) = (_value); \
    } \
    else \
    { \
        ((_pTxInfo)->Default.WIV) = (_value); \
    } \
}


#define WRITE_TXINFO_QSEL(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.QSEL) = (_value); \
    } \
    else \
    { \
        ((_pTxInfo)->Default.QSEL) = (_value); \
    } \
}


#define WRITE_TXINFO_USBDMANextVLD(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.USBDMANextVLD) = (_value); \
    } \
    else \
    { \
        ((_pTxInfo)->Default.USBDMANextVLD) = (_value); \
    } \
}


#define WRITE_TXINFO_USBDMATxburst(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.USBDMATxburst) = (_value); \
    } \
    else \
    { \
        ((_pTxInfo)->Default.USBDMATxburst) = (_value); \
    } \
}

//
// MACRO only valid before 6x9x
//
#define WRITE_TXINFO_SwUseSegIdx(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        (((PTXWI_STRUC)(_pTxInfo + 1))->Ext.SwUseSegIdx) = (_value); \
    } \
    else \
    { \
        ((_pTxInfo)->Default.SwUseSegIdx) = (_value); \
    } \
}

#define WRITE_TXINFO_SwUsepad(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        \
    } \
    else \
    { \
        ((_pTxInfo)->Default.SwUsepad) = (_value); \
    } \
}

#define WRITE_TXINFO_SwUseSegmentEnd(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.SwUseSegmentEnd) = (_value); \
    } \
    else \
    { \
        ((_pTxInfo)->Default.SwUseSegmentEnd) = (_value); \
    } \
}

#define WRITE_TXINFO_SwUseAMSDU(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.SwUseAMSDU) = (_value); \
    } \
    else \
    { \
        ((_pTxInfo)->Default.SwUseAMSDU) = (_value); \
    } \
}

#define WRITE_TXINFO_SwUseNonQoS(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.SwUseNonQoS) = (_value); \
    } \
    else \
    { \
        ((_pTxInfo)->Default.SwUseNonQoS) = (_value); \
    } \
}

//
// MACRO only valid after 6x9x
//
#define WRITE_TXINFO_Is80211(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.Is80211) = (_value); \
    } \
    else \
    { \
        \
    } \
}

#define WRITE_TXINFO_TSO(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.TSO) = (_value); \
    } \
    else \
    { \
        \
    } \
}

#define WRITE_TXINFO_CSO(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.CSO) = (_value); \
    } \
    else \
    { \
        \
    } \
}

#define WRITE_TXINFO_DPort(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.DPort) = (_value); \
    } \
    else \
    { \
        \
    } \
}

#define WRITE_TXINFO_InfoType(_pAd, _pTxInfo, _value) \
{ \
    if (EXT_MAC_CAPABLE(_pAd)) \
    { \
        ((_pTxInfo)->Ext.InfoType) = (_value); \
    } \
    else \
    { \
        \
    } \
}

//
//
//
#define READ_TXINFO_SwUseSegmentEnd(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.SwUseSegmentEnd) \
        : ((_pTxInfo)->Default.SwUseSegmentEnd))


#define READ_TXINFO_SwUseSegIdx(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? (((PTXWI_STRUC)(_pTxInfo + 1))->Ext.SwUseSegIdx) \
        : ((_pTxInfo)->Default.SwUseSegIdx))

#define READ_TXINFO_SwUseAMSDU(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.SwUseAMSDU) \
        : ((_pTxInfo)->Default.SwUseAMSDU))

#define READ_TXINFO_SwUseNonQoS(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.SwUseNonQoS) \
        : ((_pTxInfo)->Default.SwUseNonQoS))

#define READ_TXINFO_USBDMATxPktLen(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.USBDMATxPktLen) \
        : ((_pTxInfo)->Default.USBDMATxPktLen))

#define READ_TXINFO_WIV(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.WIV) \
        : ((_pTxInfo)->Default.WIV))

#define READ_TXINFO_QSEL(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.QSEL) \
        : ((_pTxInfo)->Default.QSEL))

#define READ_TXINFO_USBDMANextVLD(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.USBDMANextVLD) \
        : ((_pTxInfo)->Default.USBDMANextVLD))

#define READ_TXINFO_USBDMATxburst(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.USBDMATxburst) \
        : ((_pTxInfo)->Default.USBDMATxburst))

#define READ_TXINFO_Is80211(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.Is80211) \
        : (0))

#define READ_TXINFO_TSO(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.TSO) \
        : (0))

#define READ_TXINFO_CSO(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.CSO) \
        : (0))

#define READ_TXINFO_DPort(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.DPort) \
        : (0))

#define READ_TXINFO_InfoType(_pAd, _pTxInfo) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pTxInfo)->Ext.InfoType) \
        : (0))

//
// WRITE_TX_INFO_ macros are used to set fields of TX_INFO (Packet command)
//
#define WRITE_TX_INFO_PKT_CMD_PARAMETERS_LENGTH(_pAd, _pTxInfo, _value) \
        { \
            ((_pTxInfo)->PktCmd.PktCmdParametersLength) = (_value); \
        }

#define WRITE_TX_INFO_PKT_CMD_SEQ(_pAd, _pTxInfo, _value) \
        { \
            ((_pTxInfo)->PktCmd.PktCmdSeq) = (_value); \
        }

#define WRITE_TX_INFO_PKT_CMD_TYPE(_pAd, _pTxInfo, _value) \
        { \
            ((_pTxInfo)->PktCmd.PktCmdType) = (_value); \
        }

#define WRITE_TX_INFO_DPORT(_pAd, _pTxInfo, _value) \
        { \
            ((_pTxInfo)->PktCmd.DPort) = (_value); \
        }

#define WRITE_TX_INFO_INFO_TYPE(_pAd, _pTxInfo, _value) \
        { \
            ((_pTxInfo)->PktCmd.InfoType) = (_value); \
        }

//
//
//
#define WMM_AIFSN_CFG   0x0214

#ifdef BIG_ENDIAN
typedef union   _AIFSN_CSR_STRUC    {
    struct  {
        ULONG   Aifsn7:4;
        ULONG   Aifsn6:4;
        ULONG   Aifsn5:4;
        ULONG   Aifsn4:4;
        ULONG   Aifsn3:4;       // for AC_VO
        ULONG   Aifsn2:4;       // for AC_VI
        ULONG   Aifsn1:4;       // for AC_BK
        ULONG   Aifsn0:4;       // for AC_BE
    }   field;
    ULONG           word;
}   AIFSN_CSR_STRUC, *PAIFSN_CSR_STRUC;
#else
typedef union   _AIFSN_CSR_STRUC    {
    struct  {
        ULONG   Aifsn0:4;       // for AC_BE
        ULONG   Aifsn1:4;       // for AC_BK
        ULONG   Aifsn2:4;       // for AC_VI
        ULONG   Aifsn3:4;       // for AC_VO
        ULONG   Aifsn4:4;
        ULONG   Aifsn5:4;
        ULONG   Aifsn6:4;
        ULONG   Aifsn7:4;
    }   field;
    ULONG           word;
}   AIFSN_CSR_STRUC, *PAIFSN_CSR_STRUC;
#endif


//
// CWMIN_CSR: CWmin for each EDCA AC
//
#define WMM_CWMIN_CFG   0x0218
#ifdef BIG_ENDIAN
typedef union   _CWMIN_CSR_STRUC    {
    struct  {
        ULONG   Cwmin7:4;
        ULONG   Cwmin6:4;
        ULONG   Cwmin5:4;
        ULONG   Cwmin4:4;
        ULONG   Cwmin3:4;       // for AC_VO
        ULONG   Cwmin2:4;       // for AC_VI
        ULONG   Cwmin1:4;       // for AC_BK
        ULONG   Cwmin0:4;       // for AC_BE
    }   field;
    ULONG           word;
}   CWMIN_CSR_STRUC, *PCWMIN_CSR_STRUC;
#else
typedef union   _CWMIN_CSR_STRUC    {
    struct  {
        ULONG   Cwmin0:4;       // for AC_BE
        ULONG   Cwmin1:4;       // for AC_BK
        ULONG   Cwmin2:4;       // for AC_VI
        ULONG   Cwmin3:4;       // for AC_VO
        ULONG   Cwmin4:4;
        ULONG   Cwmin5:4;
        ULONG   Cwmin6:4;
        ULONG   Cwmin7:4;
    }   field;
    ULONG           word;
}   CWMIN_CSR_STRUC, *PCWMIN_CSR_STRUC;
#endif


//
// CWMAX_CSR: CWmin for each EDCA AC
//
#define WMM_CWMAX_CFG   0x021c
#ifdef BIG_ENDIAN
typedef union   _CWMAX_CSR_STRUC    {
    struct  {
        ULONG   Cwmax7:4;
        ULONG   Cwmax6:4;
        ULONG   Cwmax5:4;
        ULONG   Cwmax4:4;
        ULONG   Cwmax3:4;       // for AC_VO
        ULONG   Cwmax2:4;       // for AC_VI
        ULONG   Cwmax1:4;       // for AC_BK
        ULONG   Cwmax0:4;       // for AC_BE
    }   field;
    ULONG           word;
}   CWMAX_CSR_STRUC, *PCWMAX_CSR_STRUC;
#else
typedef union   _CWMAX_CSR_STRUC    {
    struct  {
        ULONG   Cwmax0:4;       // for AC_BE
        ULONG   Cwmax1:4;       // for AC_BK
        ULONG   Cwmax2:4;       // for AC_VI
        ULONG   Cwmax3:4;       // for AC_VO
        ULONG   Cwmax4:4;
        ULONG   Cwmax5:4;
        ULONG   Cwmax6:4;
        ULONG   Cwmax7:4;
    }   field;
    ULONG           word;
}   CWMAX_CSR_STRUC, *PCWMAX_CSR_STRUC;
#endif

//
// Multiple channels
//
// Firmware packet command: Tx2 -> Tx8
// Multiple channel: Tx4 -> Tx9
// Management frame: %x2 -> Tx2
// Data frame: Tx0, Tx1, Tx2, Tx3 -> Tx0, Tx1, Tx2, Tx3
//

#define WMM_CTRL    (0x0230)

#define WMM_TXOP2_CFG    (IF_DMA_REG_BASE + 0x0028)
#ifdef BIG_ENDIAN
typedef union   _AC_TXOP_CSR2_STRUC {
    struct  {
        USHORT  Ac5Txop;
        USHORT  Ac4Txop;
    }   field;
    ULONG           word;
}   AC_TXOP_CSR2_STRUC, *PAC_TXOP_CSR2_STRUC;
#else
typedef union   _AC_TXOP_CSR2_STRUC {
    struct  {
        USHORT  Ac4Txop;
        USHORT  Ac5Txop;
    }   field;
    ULONG           word;
}   AC_TXOP_CSR2_STRUC, *PAC_TXOP_CSR2_STRUC;
#endif

#define WMM_TXOP3_CFG    (IF_DMA_REG_BASE + 0x002C)
#ifdef BIG_ENDIAN
typedef union   _AC_TXOP_CSR3_STRUC {
    struct  {
        USHORT  Ac7Txop;
        USHORT  Ac6Txop;
    }   field;
    ULONG           word;
}   AC_TXOP_CSR3_STRUC, *PAC_TXOP_CSR3_STRUC;
#else
typedef union   _AC_TXOP_CSR3_STRUC {
    struct  {
        USHORT  Ac6Txop;
        USHORT  Ac7Txop;
    }   field;
    ULONG           word;
}   AC_TXOP_CSR3_STRUC, *PAC_TXOP_CSR3_STRUC;
#endif

#define MCU_CMD_CFG    ((EXT_MAC_CAPABLE(G_pAdapter)) ? (IF_DMA_REG_BASE + 0x034) : (0x022C))
#define USB_DMA_CFG    ((EXT_MAC_CAPABLE(G_pAdapter)) ? (IF_DMA_REG_BASE + 0x038) : (0x02A0))

//
// WCID Search address
//
#define WCID0_TX_RATE   0x1C00

typedef struct _FAST_RATE_CHANGE_STRUC
{
    BOOLEAN IsDataFrame;
    ULONG   WCID;
    ULONG   MacPhyRate;
} FAST_RATE_CHANGE_STRUC, *PFAST_RATE_CHANGE_STRUC;


/*

typedef union   _USB_DMA_CFG_STRUC  {
    struct // Old chips, such as 2860, 3090, 539X, 559X
    {
        ULONG  TxBusy:1;    //USB DMA TX FSM busy . debug only
        ULONG  RxBusy:1;        //USB DMA RX FSM busy . debug only
        ULONG  EpoutValid:6;        //OUT endpoint data valid. debug only
        ULONG  TxBulkEn:1;        //Enable USB DMA Tx  
        ULONG  RxBulkEn:1;        //Enable USB DMA Rx  
        ULONG  RxBulkAggEn:1;        //Enable Rx Bulk Aggregation  
        ULONG  TxopHalt:1;        //Halt TXOP count down when TX buffer is full.
        ULONG  TxClear:1;        //Clear USB DMA TX path
        ULONG  rsv:2;        
        ULONG  phyclear:1;              //phy watch dog enable. write 1
        ULONG  RxBulkAggLmt:8;        //Rx Bulk Aggregation Limit  in unit of 1024 bytes
        ULONG  RxBulkAggTOut:8;        //Rx Bulk Aggregation TimeOut  in unit of 33ns
    } Default;

    struct // New chips, such as RT6X9X
    {
        ULONG  TxBusy:1;    //USB DMA TX FSM busy . debug only
        ULONG  RxBusy:1;        //USB DMA RX FSM busy . debug only
        ULONG  Resvered:3;        //OUT endpoint data valid. debug only
        ULONG  MpdmaTxEn:1; 
        ULONG  MpdmaRxEn:1;
        ULONG  MpdmaTxClear:1;
        ULONG  TxBulkEn:1;          //Enable USB DMA Tx  
        ULONG  RxBulkEn:1;          //Enable USB DMA Rx  
        ULONG  RxBulkAggEn:1;        //Enable Rx Bulk Aggregation  
        ULONG  TxopHalt:1;        //Halt TXOP count down when TX buffer is full.
        ULONG  TxClear:1;        //Clear USB DMA TX path
        ULONG  rsv:1;
        ULONG  WakeupEn:1;
        ULONG  phyclear:1;              //phy watch dog enable. write 1
        ULONG  RxBulkAggLmt:8;        //Rx Bulk Aggregation Limit  in unit of 1024 bytes
        ULONG  RxBulkAggTOut:8;        //Rx Bulk Aggregation TimeOut  in unit of 33ns
    } Ext;

    ULONG   DoubleWord;
}   USB_DMA_CFG_STRUC, *PUSB_DMA_CFG_STRUC;

#define WRITE_USBDMA_TxBulkEn(_pAd, _pUSBDMA, _value) \
        { \
            if (EXT_MAC_CAPABLE(_pAd)) \
            { \
                ((_pUSBDMA)->Ext.TxBulkEn) = (_value); \
            } \
            else \
            { \
                ((_pUSBDMA)->Default.TxBulkEn) = (_value); \
            } \
        }

#define WRITE_USBDMA_RxBulkEn(_pAd, _pUSBDMA, _value) \
        { \
            if (EXT_MAC_CAPABLE(_pAd)) \
            { \
                ((_pUSBDMA)->Ext.RxBulkEn) = (_value); \
            } \
            else \
            { \
                ((_pUSBDMA)->Default.RxBulkEn) = (_value); \
            } \
        }

#define WRITE_USBDMA_TxClear(_pAd, _pUSBDMA, _value) \
        { \
            if (EXT_MAC_CAPABLE(_pAd)) \
            { \
                ((_pUSBDMA)->Ext.TxClear) = (_value); \
            } \
            else \
            { \
                ((_pUSBDMA)->Default.TxClear) = (_value); \
            } \
        }
*/

//
// VHT/HT Tx rate fallback configuration 0
//
#define VHT_HT_FBK_CFG0 0x1354

#ifdef BIG_ENDIAN
typedef union _VHT_HT_FBK_CFG0_STRUC
{
    struct
    {
        ULONG HTMCS7FBK:4;
        ULONG HTMCS6FBK:4;
        ULONG HTMCS5FBK:4;
        ULONG HTMCS4FBK:4;
        ULONG HTMCS3FBK:4;
        ULONG HTMCS2FBK:4;
        ULONG HTMCS1FBK:4;
        ULONG HTMCS0FBK:4;
    } field;

    ULONG word;
} VHT_HT_FBK_CFG0_STRUC, *PVHT_HT_FBK_CFG0_STRUC;
#else
typedef union _VHT_HT_FBK_CFG0_STRUC
{
    struct
    {
        ULONG HTMCS0FBK:4;
        ULONG HTMCS1FBK:4;
        ULONG HTMCS2FBK:4;
        ULONG HTMCS3FBK:4;
        ULONG HTMCS4FBK:4;
        ULONG HTMCS5FBK:4;
        ULONG HTMCS6FBK:4;
        ULONG HTMCS7FBK:4;
    } field;
    
    ULONG word;
} VHT_HT_FBK_CFG0_STRUC, *PVHT_HT_FBK_CFG0_STRUC;
#endif

//
// RF misc.
//
#define RF_MISC 0x0518

//
// VHT/HT Tx rate fallback configuration 1
//
#define VHT_HT_FBK_CFG1 0x1358

#ifdef BIG_ENDIAN
typedef union _VHT_HT_FBK_CFG1_STRUC
{
    struct
    {
        ULONG HTMCS15FBK:4;
        ULONG HTMCS14FBK:4;
        ULONG HTMCS13FBK:4;
        ULONG HTMCS12FBK:4;
        ULONG HTMCS11FBK:4;
        ULONG HTMCS10FBK:4;
        ULONG HTMCS9FBK:4;
        ULONG HTMCS8FBK:4;
    } field;
    
    ULONG word;
}   VHT_HT_FBK_CFG1_STRUC, *PVHT_HT_FBK_CFG1_STRUC;
#else
typedef union _VHT_HT_FBK_CFG1_STRUC
{
    struct
    {
        ULONG HTMCS8FBK:4;
        ULONG HTMCS9FBK:4;
        ULONG HTMCS10FBK:4;
        ULONG HTMCS11FBK:4;
        ULONG HTMCS12FBK:4;
        ULONG HTMCS13FBK:4;
        ULONG HTMCS14FBK:4;
        ULONG HTMCS15FBK:4;
    } field;
    
    ULONG word;
}   VHT_HT_FBK_CFG1_STRUC, *PVHT_HT_FBK_CFG1_STRUC;
#endif

//
// Tx Fallback limitation
//
#define TX_FBK_LIMIT      0x1398

#define VHT20_PROT_CFG  0x13E0    // VHT 20 Protection
#define VHT40_PROT_CFG  0x13E4    // VHT 40 Protection
#define VHT80_PROT_CFG  0x13E8    // VHT 80 Protection
#define PIFS_TX_CFG     0x13EC    // PIFS setting

//
// Define for the value of different MAC version
//
#define MAC_VALUE_PROTECT_FRAME_RATE_CCK_1M(_pAd)        ( (EXT_MAC_CAPABLE(_pAd)) ? (0x0000) : (0x0000) ) // CCK, 1M,  MCS0
#define MAC_VALUE_PROTECT_FRAME_RATE_CCK_11M(_pAd)       ( (EXT_MAC_CAPABLE(_pAd) ) ? (0x0003) : (0x0003) ) // CCK, 11M, MCS3
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_6M(_pAd)       ( (EXT_MAC_CAPABLE(_pAd) ) ? (0x2000) : (0x4000) ) // OFDM,6M,  MCS0
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_6M_BW40(_pAd)  ( (EXT_MAC_CAPABLE(_pAd)) ? (0x2080) : (0x4080) ) // OFDM,6M,  MCS0, BW40
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_6M_BW80(_pAd)  ( (EXT_MAC_CAPABLE(_pAd)) ? (0x2100) : (0xFFFF) ) // OFDM,6M,  MCS0, BW80
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_9M(_pAd)       ( (EXT_MAC_CAPABLE(_pAd)) ? (0x2001) : (0x4001) ) // OFDM,9M,  MCS1
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_9M_BW40(_pAd)  ( (EXT_MAC_CAPABLE(_pAd)) ? (0x2081) : (0x4081) ) // OFDM,9M,  MCS1, BW40
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_9M_BW80(_pAd)  ( (EXT_MAC_CAPABLE(_pAd)) ? (0x2101) : (0xFFFF) ) // OFDM,9M,  MCS1, BW80
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_18M(_pAd)      ( (EXT_MAC_CAPABLE(_pAd)) ? (0x2003) : (0x4003) ) // OFDM,18M, MCS3
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_18M_BW40(_pAd) ( (EXT_MAC_CAPABLE(_pAd)) ? (0x2083) : (0x4083) ) // OFDM,18M, MCS3, BW40
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_18M_BW80(_pAd) ( (EXT_MAC_CAPABLE(_pAd)) ? (0x2103) : (0xFFFF) ) // OFDM,18M, MCS3, BW80
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_36M(_pAd)      ( (EXT_MAC_CAPABLE(_pAd)) ? (0x2005) : (0x4005) ) // OFDM,36M, MCS5
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_36M_BW40(_pAd) ( (EXT_MAC_CAPABLE(_pAd)) ? (0x2085) : (0x4085) ) // OFDM,36M, MCS5, BW40
#define MAC_VALUE_PROTECT_FRAME_RATE_OFDM_36M_BW80(_pAd) ( (EXT_MAC_CAPABLE(_pAd)) ? (0x2105) : (0xFFFF) ) // OFDM,36M, MCS5, BW80


#define MAC_VALUE_MAX_LEN_CFG_DEFAULT(_pAd)   (VHT_NIC(_pAd) ? 0x003E3FFF : 0x000E2FFF) // Max MPDU, PSDU 64k (Non VHT is 32k)
#define MAC_VALUE_MAX_LEN_CFG_PSDU8K(_pAd)    (VHT_NIC(_pAd) ? 0x003E0FFF : 0x000E0FFF) // Max MPDU, PSDU 8k
#define MAC_VALUE_MAX_LEN_CFG_PSDU16K(_pAd)   (VHT_NIC(_pAd) ? 0x003E1FFF : 0x000E1FFF) // Max MPDU, PSDU 16k
#define MAC_VALUE_MAX_LEN_CFG_PSDU32K(_pAd)   (VHT_NIC(_pAd) ? 0x003E2FFF : 0x000E2FFF) // Max MPDU, PSDU 32k
#define MAC_VALUE_MAX_LEN_CFG_PSDU64K(_pAd)   (VHT_NIC(_pAd) ? 0x003E3FFF : 0x000E3FFF) // Max MPDU, PSDU 64k
#define MAC_VALUE_MAX_LEN_CFG_MINMPDU14B(_pAd)(VHT_NIC(_pAd) ? 0x003A3FFF : 0x000A2FFF) // Default Max MPDU PSDU, Min MPDU 14Byte

#define TSO_CTRL                (IF_DMA_REG_BASE + 0x0050)

#ifdef BIG_ENDIAN
typedef union _TSO_CTRL_STRUC
{
    struct
    {
        ULONG  RSV:13;
        ULONG  TSO_WR_LEN_EN:1;
        ULONG  TSO_SEG_EN:1;
        ULONG  TSO_EN:1;
        ULONG  RXWI_LEN:4;
        ULONG  RX_L2_FIX_LEN:4;
        ULONG  TXWI_LEN:4;
        ULONG  TX_L2_FIX_LEN:4;
    }   field;
    
    ULONG           word;
} TSO_CTRL_STRUC, *PTSO_CTRL_STRUC;
#else
typedef union _TSO_CTRL_STRUC
{
    struct
    {
        ULONG  TX_L2_FIX_LEN:4;
        ULONG  TXWI_LEN:4;
        ULONG  RX_L2_FIX_LEN:4;
        ULONG  RXWI_LEN:4;
        ULONG  TSO_EN:1;
        ULONG  TSO_SEG_EN:1;
        ULONG  TSO_WR_LEN_EN:1;
        ULONG  RSV:13;
    }   field;
    
    ULONG           word;
} TSO_CTRL_STRUC, *PTSO_CTRL_STRUC;
#endif

#define CSO_CTRL        0x0808
#ifdef BIG_ENDIAN
typedef union _CSO_CTRL_STRUC
{
    struct
    {
        ULONG  RSV:21;
        ULONG  STAMP_SEQ_NUM_EN:1;
        ULONG  CSR_CSO_BIGENDIAN:1;
        ULONG  CSO_EN:1;
        ULONG  TX_IPv6_EN:1;
        ULONG  TX_IPv4_CS_GEN:1;
        ULONG  TX_TCP_CS_GEN:1;
        ULONG  TX_UDP_CS_GEN:1;
        ULONG  RX_IPv6_EN:1;
        ULONG  RX_IPv4_CS_EN:1;
        ULONG  RX_TCP_CS_EN:1;
        ULONG  RX_UDP_CS_EN:1;
    } field;
    
    ULONG           word;
}   CSO_CTRL_STRUC, *PCSO_CTRL_STRUC;
#else
typedef union _CSO_CTRL_STRUC
{
    struct
    {
        ULONG  RX_UDP_CS_EN:1;
        ULONG  RX_TCP_CS_EN:1;
        ULONG  RX_IPv4_CS_EN:1;
        ULONG  RX_IPv6_EN:1;
        ULONG  TX_UDP_CS_GEN:1;
        ULONG  TX_TCP_CS_GEN:1;
        ULONG  TX_IPv4_CS_GEN:1;
        ULONG  TX_IPv6_EN:1;
        ULONG  CSO_EN:1;
        ULONG  CSR_CSO_BIGENDIAN:1;
        ULONG  STAMP_SEQ_NUM_EN:1;
        ULONG  RSV:21;
    }   field;
    
    ULONG           word;
}   CSO_CTRL_STRUC, *PCSO_CTRL_STRUC;
#endif


#define L2_STUFFING     0x080C

#ifdef BIG_ENDIAN
typedef union _L2_STUFFING_STRUC
{
    struct  {
        ULONG  RSV:6;
        ULONG  OTHER_PORT:2;
        ULONG  TS_LENGTH_EN:8;
        ULONG  TS_CMD_QSEL_EN:8;
        ULONG  RSV2:2;
        ULONG  MVINF_BYTE_SWP:1;
        ULONG  FS_WR_MPDU_LEN_EN:1;
        ULONG  TX_L2_DE_STUFFING_EN:1;
        ULONG  RX_L2_STUFFING_EN:1;
        ULONG  QoS_L2_EN:1;
        ULONG  HT_L2_EN:1;
    }   field;
    
    ULONG           word;
} L2_STUFFING_STRUC, *PL2_STUFFING_STRUC;
#else
typedef union _L2_STUFFING_STRUC
{
    struct  {
        ULONG  HT_L2_EN:1;
        ULONG  QoS_L2_EN:1;
        ULONG  RX_L2_STUFFING_EN:1;
        ULONG  TX_L2_DE_STUFFING_EN:1;
        ULONG  FS_WR_MPDU_LEN_EN:1;
        ULONG  MVINF_BYTE_SWP:1;
        ULONG  RSV2:2;
        ULONG  TS_CMD_QSEL_EN:8;
        ULONG  TS_LENGTH_EN:8;
        ULONG  OTHER_PORT:2;
        ULONG  RSV:6;
    }   field;
    
    ULONG           word;
} L2_STUFFING_STRUC, *PL2_STUFFING_STRUC;
#endif

#define MIMO_POWER_SAVE_CFG (0x1210)

//
// Point to the RXWI
//
// Note: RXWI_PTR macro must use spinlock pAd->RxRingLock.
//
#define RXWI_PTR(_pAd, _pData) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? (PRXWI_STRUC)(((PUCHAR)(_pData)) + sizeof(RX_FCE_INFO_STRUC) + sizeof(RXINFO_STRUC)) \
        : (PRXWI_STRUC)(((PUCHAR)(_pData)) + RX_DMA_LENGTH))

//
// Point to the RXINFO
//
// Note: RXINFO macro must use spinlock pAd->RxRingLock.
//
#define RXINFO_PTR(_pAd, _pData, _PacketLength) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? (PRXINFO_STRUC)(((PUCHAR)(_pData)) + sizeof(RX_FCE_INFO_STRUC)) \
        : (PRXINFO_STRUC)(((PUCHAR)(_pData)) + RX_DMA_LENGTH  + _PacketLength))
        //: (PRXINFO_STRUC)(((PUCHAR)(_pData)) + RX_DMA_LENGTH + _pAd->HwCfg.RXWI_Length + _PacketLength))

//
// Point to the 802.11 header
//
// Note: DOT11_HEADER_PTR macro must use spinlock pAd->RxRingLock.
//
#define DOT11_HEADER_PTR(_pAd, _pData) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? (PHEADER_802_11)(((PUCHAR)(_pData)) + sizeof(RX_FCE_INFO_STRUC) + sizeof(RXINFO_STRUC) + _pAd->HwCfg.RXWI_Length) \
        : (PHEADER_802_11)(((PUCHAR)(_pData)) + RX_DMA_LENGTH + _pAd->HwCfg.RXWI_Length))
        


        
//
// Point to the RX_FCE_INFO
//
//Note: RX_FCE_INFO_PTR macro must use spinlock pAd->RxRingLock.
//
//#define RX_FCE_INFO_PTR(_pAd, _RxRingIdx) ((PRX_FCE_INFO_STRUC)(((PUCHAR)(_pAd->RxRing.Cell[_RxRingIdx].AllocVa)) + sizeof(RXD_STRUC)))


//
// TXWI
//
//  1. Do not access the field directly
//  2. Use READ_TXWI_XXX and WRITE_TXWI_XXX macros to get and set fields of TXWI
//
typedef union   _TXWI_STRUC
{
    struct // The TXWI of old chips, such as 2860, 3090, 539X, 559X
    {
        //
        // Word 0
        //
        ULONG       Frag:1; // 1 to inform TKIP engine this is a fragment.
        ULONG       MimoPs:1; // The remote peer is in dynamic MIMO-PS mode
        ULONG       CfAck:1; // DATA+CFACK
        ULONG       TS:1; // Add timestamp for beacon for probe response
        ULONG       AMPDU:1; // AMPDU
        ULONG       MpduDensity:3; // MPDU density
        ULONG       TXOP:2; //FOR "THIS" frame. 0:HT TXOP rule , 1:PIFS TX ,2:Backoff, 3:sifs only when previous frame exchange is successful.
        ULONG       NdpStream:2; // Number of streams in NDP, {0: 1 stream, 1: 2 streams, 2: 3 streams, 3: 4 streams}
        ULONG       NdpBw:1; // NDP bandwidth, {0: 20MHz, 1: 40MHz}
        ULONG       TXFBK:1; // Disable Tx auto fallback for this frame, {1: disable, 0: follow the register settings}
        ULONG       TxRprt:1; // Tx reporting tag, {1: TX_REPORT_CNT increase by one, 0: do nothing}
        ULONG       Reserved1:1; // Reserved
        ULONG       MCS:7; // MCS
        ULONG       BW:1; // Bandwidth
        ULONG       ShortGI:1; // Short GI
        ULONG       STBC:2; // STBC
        ULONG       ETxBf:1; // Explicit TxBF
        ULONG       Sounding:1; // Sounding frame
        ULONG       ITxBf:1; // Implicit TxBF
        ULONG       PhyMode:2; // PHY mode

        //
        // Word 1
        //
        ULONG       Ack:1; // Inform MAC to wait for ACK or not after transmission
        ULONG       NSEQ:1; // Special hardware sequence number register in MAC block
        ULONG       BaWindowSize:6; // BA window size
        ULONG       WCID:8; // WCID
        ULONG       MpduTotalByteCount:12; // Total frame length
        ULONG       PacketId:4; // Packet ID by driver

        //
        //Word2
        //
        ULONG       IV; // Used by encryption engine

        //
        //Word3
        //
        ULONG       EIV; // Used by encryption engine

        //
        // for Expert Antenna
        //
        //Word4
        ULONG       Reserved2; // Reserved
    } Default;

    struct // The TXWI of new chips, such as RT6X9X
    {
        //
        // Word 0
        //
        ULONG       Frag:1; // 1 to inform TKIP engine this is a fragment.
        ULONG       MimoPs:1; // The remote peer is in dynamic MIMO-PS mode
        ULONG       CfAck:1; // DATA+CFACK
        ULONG       TS:1; // Add timestamp for beacon for probe response
        ULONG       AMPDU:1; // AMPDU
        ULONG       MpduDensity:3; // MPDU density
        ULONG       TXOP:2; //FOR "THIS" frame. 0:HT TXOP rule , 1:PIFS TX ,2:Backoff, 3:sifs only when previous frame exchange is successful.
        ULONG       NdpStream:2; // Number of streams in NDP, {0: 1 stream, 1: 2 streams, 2: 3 streams, 3: 4 streams}
        ULONG       NdpBw:2; // NDP bandwidth, {0: 20MHz, 1: 40MHz, 2: 80MHz}
        ULONG       Sounding:1; // Sounding frame
        ULONG       LutEn:1;    // Lookup Rate Table policy for fast rate changing, it's faster then rate change by TxWI. 

        //
        //  1. CCK/OFDM: 
        //      McsNss[6:0] = MCS (0~7)
        //
        //  2. HT: 
        //      McsNss[2:0] = MCS (0~7)
        //      McsNss[6:3] = Nss (0~1)
        //
        //  3. VHT: 
        //      McsNss[3:0] = MCS (0~7)
        //      McsNss[6:4] = Nss (0~1)
        //
        //  4. Not support MCS 32
        //
        //  5. Not support VHT MCS 8/9
        //
        ULONG       McsNss:7;
        
        ULONG       BW:2; // Bandwidth
        ULONG       ShortGI:1; // Short GI
        ULONG       STBC:1; // STBC
        ULONG       ETxBf:1; // Explicit TxBF
        ULONG       ITxBf:1; // Implicit TxBF
        ULONG       PhyMode:3; // PHY mode

        //
        // Word 1
        //
        ULONG       Ack:1; // Inform MAC to wait for ACK or not after transmission
        ULONG       NSEQ:1; // Special hardware sequence number register in MAC block
        ULONG       BaWindowSize:6; // BA window size
        ULONG       WCID:8; // WCID

        ULONG       MpduTotalByteCount:14; // Total frame length
        ULONG       Reserved2:2; // Reserved

        //
        //Word2
        //
        ULONG       IV; // Used by encryption engine

        //
        //Word3
        //
        ULONG       EIV; // Used by encryption engine

        //
        //Word4
        //
        ULONG       TxEapId:8; // Tx EAP ID
        ULONG       TxStreamMode:8; // Tx stream mode
        ULONG       TxPowerAdjust:4; // Tx power adjustment
        //ULONG     Reserved3:4; // Reserved, used as SwUseSegIdx which it should be in TXINFO
        ULONG       SwUseSegIdx:4;
        ULONG       PacketId:8; // Tx packet ID
    } Ext;
}   TXWI_STRUC, *PTXWI_STRUC;

typedef struct _MAC_PHYRATE_STRUC
{
    // The Mac Phy rate of new chips, such as RT6X9X
    ULONG       McsNss:7;    // MCS and Nss
                             // For HT mode,  [2:0]=MCS, [6:3]=Nss
                             // For VHT mode, [3:0]=MCS, [6:4]=Nss                           
    ULONG       BW:2;        // Bandwidth, 0:20M, 1:40M, 2:80M
    ULONG       ShortGI:1;  // Short GI
    ULONG       STBC:1;     // STBC
    ULONG       Reserved:2;  // Reserved
    ULONG       PhyMode:3;   // PHY mode, MODE_VHT/ MODE_HTMIX/ ... 

}   MAC_PHYRATE_STRUC, *PMAC_PHYRATE_STRUC;

//
// READ_TXWI_XXX and WRITE_TXWI_XXX macros are used to get and set fields of TXWI
//

// MT7601 has rollbacked lay out as 5592
#define EXT_TWXI_RXWI_ENABLE(_pAd) (EXT_MAC_CAPABLE(_pAd))


#define READ_TXWI_FRAG(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.Frag) \
        : ((_pTxWI)->Default.Frag))

#define READ_TXWI_MIMO_PS(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.MimoPs) \
        : ((_pTxWI)->Default.MimoPs))

#define READ_TXWI_CFACK(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.CfAck) \
        : ((_pTxWI)->Default.CfAck))

#define READ_TXWI_TS(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.TS) \
        : ((_pTxWI)->Default.TS))

#define READ_TXWI_AMPDU(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.AMPDU) \
        : ((_pTxWI)->Default.AMPDU))

#define READ_TXWI_MPDU_DENSITY(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.MpduDensity) \
        : ((_pTxWI)->Default.MpduDensity))

#define READ_TXWI_TXOP(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.TXOP) \
        : ((_pTxWI)->Default.TXOP))

#define READ_TXWI_NDP_STREAM(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.NdpStream) \
        : ((_pTxWI)->Default.NdpStream))

#define READ_TXWI_NDP_BW(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.NdpBw) \
        : ((_pTxWI)->Default.NdpBw))

#define READ_TXWI_TX_AUTO_FALLBACK(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.TXFBK) \
        : ((_pTxWI)->Default.TXFBK))

#define READ_TXWI_TX_REPORT_TAG(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.TxRprt) \
        : ((_pTxWI)->Default.TxRprt))

//
//  CCK: MCS 0, 1, 2, 3, 8, 9, 10 and 11
//  OFDM: MCS 0~7
//  HT: MCS 0~23
//  VHT: MCS 0~7
//
#define READ_TXWI_MCS(_pAd, _pTxWI, _Mode) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((((_Mode) == MODE_CCK) || ((_Mode) == MODE_OFDM)) ? ((_pTxWI)->Ext.McsNss) : (((_Mode) == MODE_VHT) ? ((_pTxWI)->Ext.McsNss & 0x0F) : (((((_pTxWI)->Ext.McsNss & 0x78) >> 3) * 8) + (((_pTxWI)->Ext.McsNss & 0x07) % 8)))) \
        : ((_pTxWI)->Default.MCS))

//
//  CCK: N/A
//  OFDM: N/A
//  HT: Nss 0~1
//  VHT: Nss 0~1
//
#define READ_TXWI_NSS(_pAd, _pTxWI, _Mode) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? (((_Mode) == MODE_VHT) ? (((_pTxWI)->Ext.McsNss & 0x70) >> 4) : ((((_Mode) == MODE_HTMIX) || ((_Mode) == MODE_HTGREENFIELD)) ? (((_pTxWI)->Ext.McsNss & 0x78) >> 3) : (0))) \
        : ((_pTxWI)->Default.MCS))

#define READ_TXWI_BW(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.BW) \
        : ((_pTxWI)->Default.BW))

#define READ_TXWI_SHORT_GI(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.ShortGI) \
        : ((_pTxWI)->Default.ShortGI))

#define READ_TXWI_STBC(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.STBC) \
        : ((_pTxWI)->Default.STBC))

#define READ_TXWI_EXTBF(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.ETxBf) \
        : ((_pTxWI)->Default.ETxBf))

#define READ_TXWI_SOUNDING(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.Sounding) \
        : ((_pTxWI)->Default.Sounding))

#define READ_TXWI_ITXBF(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.ITxBf) \
        : ((_pTxWI)->Default.ITxBf))

#define READ_TXWI_PHY_MODE(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.PhyMode) \
        : ((_pTxWI)->Default.PhyMode))

#define READ_TXWI_ACK(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.Ack) \
        : ((_pTxWI)->Default.Ack))

#define READ_TXWI_NSEQ(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.NSEQ) \
        : ((_pTxWI)->Default.NSEQ))

#define READ_TXWI_BA_WINDOW_SIZE(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.BaWindowSize) \
        : ((_pTxWI)->Default.BaWindowSize))

#define READ_TXWI_WCID(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.WCID) \
        : ((_pTxWI)->Default.WCID))

#define READ_TXWI_MPDU_TOTAL_BYTE_COUNT(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.MpduTotalByteCount) \
        : ((_pTxWI)->Default.MpduTotalByteCount))

#define READ_TXWI_PACKET_ID(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.PacketId) \
        : ((_pTxWI)->Default.PacketId))

#define READ_TXWI_IV(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.IV) \
        : ((_pTxWI)->Default.IV))

#define READ_TXWI_EIV(_pAd, _pTxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pTxWI)->Ext.EIV) \
        : ((_pTxWI)->Default.EIV))

#define WRITE_TXWI_FRAG(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.Frag) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.Frag) = (_value); \
            } \
        }

#define WRITE_TXWI_MIMO_PS(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.MimoPs) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.MimoPs) = (_value); \
            } \
        }

#define WRITE_TXWI_CFACK(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.CfAck) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.CfAck) = (_value); \
            } \
        }

#define WRITE_TXWI_TS(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.TS) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.TS) = (_value); \
            } \
        }

#define WRITE_TXWI_AMPDU(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.AMPDU) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.AMPDU) = (_value); \
            } \
        }

#define WRITE_TXWI_MPDU_DENSITY(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.MpduDensity) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.MpduDensity) = (_value); \
            } \
        }

#define WRITE_TXWI_TXOP(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.TXOP) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.TXOP) = (_value); \
            } \
        }

#define WRITE_TXWI_NDP_STREAM(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.NdpStream) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.NdpStream) = (_value); \
            } \
        }

#define WRITE_TXWI_NDP_BW(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.NdpBw) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.NdpBw) = (_value); \
            } \
        }

#define WRITE_TXWI_TX_AUTO_FALLBACK(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.TXFBK) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.TXFBK) = (_value); \
            } \
        }

#define WRITE_TXWI_TX_REPORT_TAG(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.TxRprt) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.TxRprt) = (_value); \
            } \
        }

#define WRITE_TXWI_LUT_EN(_pAd, _pTxWI, _value) \
        { \
            if (EXT_MAC_CAPABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.LutEn) = (_value); \
            } \
            else \
            { \
                ; \
            } \
        }

//
//  CCK: MCS 0, 1, 2, 3, 8, 9, 10 and 11
//  OFDM: MCS 0~7
//  HT: MCS 0~23
//  VHT: MCS 0~7
//
#define WRITE_TXWI_MCS(_pAd, _pTxWI, _Mode, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                if (((_Mode) == MODE_HTMIX) || ((_Mode) == MODE_HTGREENFIELD)) /* HT */ \
                { \
                    ((_pTxWI)->Ext.McsNss) = ((((_value) / 8) << 3) | ((_value) % 8)); /* Nss+MCS */ \
                } \
                else if ((_Mode) == MODE_VHT) /* VHT */ \
                { \
                    ((_pTxWI)->Ext.McsNss) |= (_value); /* MCS */ \
                } \
                else /* CCK/OFDM */ \
                { \
                    ((_pTxWI)->Ext.McsNss) = (_value); /* MCS */ \
                } \
            } \
            else \
            { \
                ((_pTxWI)->Default.MCS) = (_value); \
            } \
        }

//
//  CCK: N/A
//  OFDM: N/A
//  HT: N/A
//  VHT: Nss 0~1
//
#define WRITE_TXWI_NSS(_pAd, _pTxWI, _Mode, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                if ((_Mode) == MODE_VHT) /* VHT */ \
                { \
                    ((_pTxWI)->Ext.McsNss) |= ((_value) << 4); \
                } \
                else /* CCK/OFDM/HT */ \
                { \
                    /* Do nothing */ \
                } \
            } \
            else \
            { \
                /* Do nothing */ \
            } \
        }

#define WRITE_TXWI_BW(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.BW) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.BW) = (_value); \
            } \
        }

#define WRITE_TXWI_SHORT_GI(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.ShortGI) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.ShortGI) = (_value); \
            } \
        }

#define WRITE_TXWI_STBC(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.STBC) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.STBC) = (_value); \
            } \
        }

#define WRITE_TXWI_EXTBF(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.ETxBf) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.ETxBf) = (_value); \
            } \
        }

#define WRITE_TXWI_SOUNDING(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.Sounding) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.Sounding) = (_value); \
            } \
        }

#define WRITE_TXWI_ITXBF(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.ITxBf) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.ITxBf) = (_value); \
            } \
        }

#define WRITE_TXWI_PHY_MODE(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.PhyMode) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.PhyMode) = (_value); \
            } \
        }

#define WRITE_TXWI_ACK(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.Ack) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.Ack) = (_value); \
            } \
        }

#define WRITE_TXWI_NSEQ(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.NSEQ) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.NSEQ) = (_value); \
            } \
        }

#define WRITE_TXWI_BA_WINDOW_SIZE(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.BaWindowSize) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.BaWindowSize) = (_value); \
            } \
        }

#define WRITE_TXWI_WCID(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.WCID) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.WCID) = (_value); \
            } \
        }

#define WRITE_TXWI_MPDU_TOTAL_BYTE_COUNT(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.MpduTotalByteCount) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.MpduTotalByteCount) = (_value); \
            } \
        }

#define WRITE_TXWI_PACKET_ID(_pAd, _pTxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.PacketId) = (_value); \
            } \
            else \
            { \
                ((_pTxWI)->Default.PacketId) = (_value); \
            } \
        }

#define WRITE_TXWI_IV(_pAd, _pTxWI, _pValue) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                PlatformMoveMemory(&((_pTxWI)->Ext.IV), (_pValue), sizeof(ULONG)); \
            } \
            else \
            { \
                PlatformMoveMemory(&((_pTxWI)->Default.IV), (_pValue), sizeof(ULONG)); \
            } \
        }

#define WRITE_TXWI_EIV(_pAd, _pTxWI, _pValue) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                PlatformMoveMemory(&((_pTxWI)->Ext.EIV), (_pValue), sizeof(ULONG)); \
            } \
            else \
            { \
                PlatformMoveMemory(&((_pTxWI)->Default.EIV), (_pValue), sizeof(ULONG)); \
            } \
        }

#define WRITE_TXWI_TX_POWER_ADJUST(_pAd, _pTxWI, _value) \
        { \
            if (EXT_MAC_CAPABLE(_pAd)) \
            { \
                ((_pTxWI)->Ext.TxPowerAdjust) = (_value); \
            } \
        }

//
// RXWI
//
//  1. Do not access the field directly
//  2. Use READ_RXWI_XXX and WRITE_RXWI_XXX macros to get and set fields of RXWI
//
typedef union _RXWI_STRUC
{
    struct // The RXWI of old chips, such as 2860, 3090, 539X, 559X
    {
        //
        // Word 0
        //
        ULONG   WCID:8; // WCID and WCID=0xFF means "not found"
        ULONG   KeyIdx:2; // Key index, bit2:bit0
        ULONG   BssIdx:3; // BSSID 0~7
        ULONG   UDF:3; // User defined field
        ULONG   MpduTotalByteCount:12; // The entire MPDU length
        ULONG   TID:4; // TID from 802.11 QoS field

        //
        // Word 1
        //
        ULONG   FN:4; // Fragmented number from 802.11 header
        ULONG   SN:12; // Sequence number
        ULONG   MCS:7; // MCS from PLCP header
        ULONG   BW:1; // Bandwidth from PLCP header
        ULONG   ShortGI:1; // Short GI from PLCP header
        ULONG   STBC:2; // STBC from PLCP header
        ULONG   ETxBf:1; // Explicit TxBF
        ULONG   Sounding:1; // Sounding frame
        ULONG   ITxBf:1; // Implicit TxBF
        ULONG   PhyMode:2; // PHY mode

        //
        // Word 2
        //
        ULONG   Rssi0:8; // RSSI 0
        ULONG   Rssi1:8; // RSSI 1
        ULONG   Rssi2:8; // RSSI 2
        ULONG   Reserved1:8; // Resverved 1

        //
        // Word 3
        //
        ULONG   Snr0:8; // SNR 0
        ULONG   Snr1:8; // SNR 1
        ULONG   Snr2:8; // SNR 2
        ULONG   FreqOffset:8; // BBP reported frequency offset of the received frame

        //
        // Word 4, added for rt3593
        //
        ULONG   RssiAnt0:8; // BBP reported RSSI on antenna 0 of the received frame
        ULONG   BfSnr0:8; // BBP reported TxBF SNR of the received frame
        ULONG   BfSnr1:8; // BBP reported TxBF SNR of the received frame
        ULONG   BfSnr2:8; // BBP reported TxBF SNR of the received frame

        //
        // Word 5, for Expert Antenna
        //
        ULONG   Reserved2; // Reserved 2
        
    } Default;

    struct // The RXWI of new chips, such as RT6X9X
    {
        //
        // Word 0
        //
        ULONG   WCID:8; // WCID and WCID=0xFF means "not found"
        ULONG   KeyIdx:2; // Key index, bit2:bit0
        ULONG   BssIdx:3; // BSSID 0~7
        ULONG   UDF:3; // User defined field
        ULONG   MpduTotalByteCount:14; // The entire MPDU length
        ULONG   Reserved1:1; // Reserved
        ULONG   Eof:1; // EOF

        //
        // Word 1
        //
        ULONG   TID:4; // TID from 802.11 QoS field
        ULONG   SN:12; // Sequence number

        //
        //  1. CCK/OFDM: 
        //      McsNss[6:0] = MCS (0~7)
        //
        //  2. HT: 
        //      McsNss[2:0] = MCS (0~7)
        //      McsNss[6:3] = Nss (0~1)
        //
        //  3. VHT: 
        //      McsNss[3:0] = MCS (0~7)
        //      McsNss[6:4] = Nss (0~1)
        //
        //  4. Not support MCS 32
        //
        //  5. Not support VHT MCS 8/9
        //
        ULONG   McsNss:7;
        
        ULONG   BW:2; // Bandwidth from PLCP header
        ULONG   ShortGI:1; // Short GI from PLCP header
        ULONG   STBC:1; // STBC from PLCP header
        ULONG   ETxBf:1; // Explicit TxBF
        ULONG   ITxBf:1; // Implicit TxBF
        ULONG   PhyMode:3; // PHY mode

        //
        // Word 2
        //
        ULONG   Rssi0:8; // RSSI 0
        ULONG   Rssi1:8; // RSSI 1
        ULONG   Rssi2:8; // RSSI 2
        ULONG   Rssi3:8; // RSSI 3

        //
        // Word 3
        //
        ULONG   BbpRxInfo0:8; // BBP RXINFO 0
        ULONG   BbpRxInfo1:8; // BBP RXINFO 1
        ULONG   BbpRxInfo2:8; // BBP RXINFO 2
        ULONG   BbpRxInfo3:8; // BBP RXINFO 3

        //
        // Word 4
        //
        ULONG   BbpRxInfo4:8; // BBP RXINFO 4
        ULONG   BbpRxInfo5:8; // BBP RXINFO 5
        ULONG   BbpRxInfo6:8; // BBP RXINFO 6
        ULONG   BbpRxInfo7:8; // BBP RXINFO 7

        //
        // Word 5
        //
        ULONG   BbpRxInfo8:8; // BBP RXINFO 8
        ULONG   BbpRxInfo9:8; // BBP RXINFO 9
        ULONG   BbpRxInfo10:8; // BBP RXINFO 10
        ULONG   BbpRxInfo11:8; // BBP RXINFO 11

        //
        // Word 6
        //
        ULONG   BbpRxInfo12:8; // BBP RXINFO 12
        ULONG   BbpRxInfo13:8; // BBP RXINFO 13
        ULONG   BbpRxInfo14:8; // BBP RXINFO 14

        ULONG ScrambleBwRsv:4;
        ULONG ScrambleBwType:1;
        ULONG ScrambleBwValue:2;
        ULONG :1;

        //ULONG BbpRxInfo15:8; // BBP RXINFO 15

    } Ext;
}   RXWI_STRUC, *PRXWI_STRUC;



//
// READ_RXWI_XXX and WRITE_RXWI_XXX macros are used to get and set fields of RXWI
//
#define READ_RXWI_WCID(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.WCID) \
        : ((_pRxWI)->Default.WCID))

#define READ_RXWI_KEY_IDX(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.KeyIdx) \
        : ((_pRxWI)->Default.KeyIdx))

#define READ_RXWI_BSS_IDX(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.BssIdx) \
        : ((_pRxWI)->Default.BssIdx))

#define READ_RXWI_UDF(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.UDF) \
        : ((_pRxWI)->Default.UDF))

#define READ_RXWI_MPDU_TOTAL_BYTE_COUNT(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.MpduTotalByteCount) \
        : ((_pRxWI)->Default.MpduTotalByteCount))

#define READ_RXWI_TID(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.TID) \
        : ((_pRxWI)->Default.TID))

#define READ_RXWI_FN(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.FN) \
        : ((_pRxWI)->Default.FN))

#define READ_RXWI_SN(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.SN) \
        : ((_pRxWI)->Default.SN))

//
//  CCK: MCS 0, 1, 2, 3, 8, 9, 10 and 11
//  OFDM: MCS 0~7
//  HT: MCS 0~23
//  VHT: MCS 0~7
//
#define READ_RXWI_MCS(_pAd, _pRxWI, _Mode) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((((_Mode) == MODE_CCK) || ((_Mode) == MODE_OFDM)) ? ((_pRxWI)->Ext.McsNss) : (((_Mode) == MODE_VHT) ? ((_pRxWI)->Ext.McsNss & 0x0F) : (((((_pRxWI)->Ext.McsNss & 0x78) >> 3) * 8) + (((_pRxWI)->Ext.McsNss & 0x07) % 8)))) \
        : ((_pRxWI)->Default.MCS))

//
//  CCK: N/A
//  OFDM: N/A
//  HT: Nss 0~1
//  VHT: Nss 0~1
//
// Note: 7650/7630 report incorrect Nss if the received packet with STBC is enabled.
//
#define READ_RXWI_NSS(_pAd, _pRxWI, _Mode) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? (((_Mode) == MODE_VHT) ? ((((_pRxWI)->Ext.McsNss & 0x70) >> 4) - _pRxWI->Ext.STBC) : ((((_Mode) == MODE_HTMIX) || ((_Mode) == MODE_HTGREENFIELD)) ? (((_pRxWI)->Ext.McsNss & 0x78) >> 3) : (0))) \
        : ((_pRxWI)->Default.MCS))

#define READ_RXWI_BW(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.BW) \
        : ((_pRxWI)->Default.BW))

#define READ_RXWI_SHORT_GI(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.ShortGI) \
        : ((_pRxWI)->Default.ShortGI))

#define READ_RXWI_STBC(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.STBC) \
        : ((_pRxWI)->Default.STBC))

#define READ_RXWI_ETXBF(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.ETxBf) \
        : ((_pRxWI)->Default.ETxBf))

#define READ_RXWI_SOUNDING(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.Sounding) \
        : ((_pRxWI)->Default.Sounding))

#define READ_RXWI_ITXBF(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.ITxBf) \
        : ((_pRxWI)->Default.ITxBf))

#define READ_RXWI_PHY_MODE(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.PhyMode) \
        : ((_pRxWI)->Default.PhyMode))

#define READ_RXWI_RSSI0(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.Rssi0) \
        : ((_pRxWI)->Default.Rssi0))

#define READ_RXWI_RSSI1(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.Rssi1) \
        : ((_pRxWI)->Default.Rssi1))

#define READ_RXWI_RSSI2(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.Rssi2) \
        : ((_pRxWI)->Default.Rssi2))

#define READ_RXWI_AntSel(_pAd, _pRxWI) \
        (0)
        
//2 TODO: How to get SNR0?
#define READ_RXWI_SNR0(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.BbpRxInfo2) \
        : ((_pRxWI)->Default.Snr0))
//2 TODO: How to get SNR1?
#define READ_RXWI_SNR1(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.BbpRxInfo3) \
        : ((_pRxWI)->Default.Snr1))
//2 TODO: How to get SNR2?
#define READ_RXWI_SNR2(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.BbpRxInfo4) \
        : ((_pRxWI)->Default.Snr2))
//2 TODO: How to get frequency offset
#define READ_RXWI_FREQ_OFFSET(_pAd, _pRxWI) \
        ((EXT_TWXI_RXWI_ENABLE(_pAd)) \
        ? ((_pRxWI)->Ext.BbpRxInfo1) \
        : ((_pRxWI)->Default.FreqOffset))

#define READ_RXWI_SCRAMBLEBWTYPE(_pAd, _pRxWI) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pRxWI)->Ext.ScrambleBwType) \
        : (0))

#define READ_RXWI_SCRAMBLEBWVALUE(_pAd, _pRxWI) \
        ((EXT_MAC_CAPABLE(_pAd)) \
        ? ((_pRxWI)->Ext.ScrambleBwValue) \
        : (0))

#define WRITE_RXWI_WCID(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.WCID) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.WCID) = (_value); \
            } \
        }

#define WRITE_RXWI_KEY_IDX(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.KeyIdx) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.KeyIdx) = (_value); \
            } \
        }

#define WRITE_RXWI_BSS_IDX(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.BssIdx) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.BssIdx) = (_value); \
            } \
        }

#define WRITE_RXWI_UDF(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.UDF) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.UDF) = (_value); \
            } \
        }

#define WRITE_RXWI_MPDU_TOTAL_BYTE_COUNT(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.MpduTotalByteCount) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.MpduTotalByteCount) = (_value); \
            } \
        }

#define WRITE_RXWI_TID(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.TID) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.TID) =(_value); \
            } \
        }

#define WRITE_RXWI_FN(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.FN) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.FN) = (_value); \
            } \
        }

#define WRITE_RXWI_SN(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.SN) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.SN) = (_value); \
            } \
        }

//
//  CCK: MCS 0, 1, 2, 3, 8, 9, 10 and 11
//  OFDM: MCS 0~7
//  HT: MCS 0~23
//  VHT: MCS 0~7
//
#define WRITE_RXWI_MCS(_pAd, _pRxWI, _Mode, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                if (((_Mode) == MODE_HTMIX) || ((_Mode) == MODE_HTGREENFIELD)) /* HT */ \
                { \
                    ((_pRxWI)->Ext.McsNss) = ((((_value) / 8) << 3) | ((_value) % 8)); /* Nss+MCS */ \
                } \
                else if ((_Mode) == MODE_VHT) /* VHT */ \
                { \
                    ((_pRxWI)->Ext.McsNss) |= (_value); /* MCS */ \
                } \
                else /* CCK/OFDM */ \
                { \
                    ((_pRxWI)->Ext.McsNss) = (_value); /* MCS */ \
                } \
            } \
            else \
            { \
                ((_pRxWI)->Default.MCS) = (_value); \
            } \
        }

//
//  CCK: N/A
//  OFDM: N/A
//  HT: N/A
//  VHT: Nss 0~1
//
#define WRITE_RXWI_NSS(_pAd, _pRxWI, _Mode, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                if ((_Mode) == MODE_VHT) /* VHT */ \
                { \
                    ((_pRxWI)->Ext.McsNss) |= ((_value) << 4); \
                } \
                else /* CCK/OFDM/HT */ \
                { \
                    /* Do nothing */ \
                } \
            } \
            else \
            { \
                /* Do nothing */ \
            } \
        }

#define WRITE_RXWI_BW(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.BW) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.BW) = (_value); \
            } \
        }

#define WRITE_RXWI_SHORT_GI(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.ShortGI) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.ShortGI) = (_value); \
            } \
        }

#define WRITE_RXWI_STBC(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.STBC) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.STBC) = (_value); \
            } \
        }

#define WRITE_RXWI_ETXBF(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.ETxBf) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.ETxBf) = (_value); \
            } \
        }

#define WRITE_RXWI_SOUNDING(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.Sounding) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.Sounding) = (_value); \
            } \
        }

#define WRITE_RXWI_ITXBF(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.ITxBf) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.ITxBf) = (_value); \
            } \
        }

#define WRITE_RXWI_PHY_MODE(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.PhyMode) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.PhyMode) = (_value); \
            } \
        }

#define WRITE_RXWI_RSSI0(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.Rssi0) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.Rssi0) = (_value); \
            } \
        }

#define WRITE_RXWI_RSSI1(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.Rssi1) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.Rssi1) = (_value); \
            } \
        }

#define WRITE_RXWI_RSSI2(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.Rssi2) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.Rssi2) = (_value); \
            } \
        }
//2 TODO: How to get SNR0?
#define WRITE_RXWI_SNR0(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.Rssi0) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.Snr0) = (_value); \
            } \
        }
//2 TODO: How to get SNR1?
#define WRITE_RXWI_SNR1(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.Rssi1) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.Snr1) = (_value); \
            } \
        }
//2 TODO: How to get SNR2?
#define WRITE_RXWI_SNR2(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.Rssi2) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.Snr2) = (_value); \
            } \
        }
//2 TODO: How to get frequency offset?
#define WRITE_RXWI_FREQ_OFFSET(_pAd, _pRxWI, _value) \
        { \
            if (EXT_TWXI_RXWI_ENABLE(_pAd)) \
            { \
                ((_pRxWI)->Ext.Rssi0) = (_value); \
            } \
            else \
            { \
                ((_pRxWI)->Default.FreqOffset) = (_value); \
            } \
        }


//
// Port types (PORT_TYPE_XXX)
//
#define PORT_TYPE_WLAN_PORT             (0)
#define PORT_TYPE_CPU_RX_PORT           (1)
#define PORT_TYPE_CPU_TX_PORT           (2)
#define PORT_TYPE_HOST_PORT             (3)
#define PORT_TYPE_VIRTUAL_CPU_RX_PORT   (4)
#define PORT_TYPE_VIRTUAL_CPU_TX_PORT   (5)
#define PORT_TYPE_DISCARD               (6)

//
// TX_FCE_INFO_STRUC (802.11 packet or MCU event)
//
//  1. Do not access the field directly
//  2. Use READ_TX_FCE_INFO_XXX get the fields of RX_FCE_INFO_STRUC
//
typedef struct _TX_FCE_INFO_STRUC
{
    //union
    //{
        //struct
        //{
            ULONG   PKT_LEN:14;
            ULONG   reserved1:5;
            ULONG   Is80211:1;
            ULONG   reserved2:4;
            ULONG   WIV:1;
            ULONG   QSEL:2;
            ULONG   D_PORT:3;
            ULONG   InfoType:2; // Information type (INFO_TYPE_XXX)
        //} Dot11PktInfo;
        /*
        struct // Response event
        {
            ULONG   PktLength:13; // Packet length
            ULONG   Reserved1:1; // Reserved
            ULONG   Reserved2:1; // Reserved
            ULONG   SelfGenRspEvt:1; // Self-generated response event
            ULONG   CmdSeq:4; // Packet command sequence
            ULONG   RspEvtType:4; // Response event type (RSP_EVT_TYPE_XXX)
            ULONG   Reserved3:1; // Reserved
            ULONG   Reserved4:2; // Reserved
            ULONG   SPort:3; // Which S-Port (PORT_TYPE_XXX)
            ULONG   InfoType:2; // Information type (INFO_TYPE_XXX)
        } RspEvt; // Response event
        */
    //} ParameterData;  
} TX_FCE_INFO_STRUC, *PTX_FCE_INFO_STRUC;

//
// RX_FCE_INFO_STRUC (802.11 packet or MCU event)
//
//  1. Do not access the field directly
//  2. Use READ_RX_FCE_INFO_XXX get the fields of RX_FCE_INFO_STRUC
//
typedef struct _RX_FCE_INFO_STRUC
{
    union
    {
        struct
        {
            ULONG   PKT_LEN:14;
            ULONG   reserved1:1;
            ULONG   reserved2:1;
            ULONG   UDP_ERR:1;
            ULONG   TCP_ERR:1;
            ULONG   IP_ERR:1;
            ULONG   reserved3:1;            
            ULONG   L3L4_DONE:1;
            ULONG   MAC_LEN:3;
            ULONG   PCIe_INT:1;
            ULONG   QSEL:2;
            ULONG   S_PORT:3;
            ULONG   InfoType:2; // Information type (INFO_TYPE_XXX)
        } Dot11PktInfo;     

        struct // Response event
        {
            ULONG   PktLength:13; // Packet length
            ULONG   Reserved1:1; // Reserved
            ULONG   Reserved2:1; // Reserved
            ULONG   SelfGenRspEvt:1; // Self-generated response event
            ULONG   CmdSeq:4; // Packet command sequence
            ULONG   RspEvtType:4; // Response event type (RSP_EVT_TYPE_XXX)
            ULONG   Reserved3:1; // Reserved
            ULONG   Reserved4:2; // Reserved
            ULONG   SPort:3; // Which S-Port (PORT_TYPE_XXX)
            ULONG   InfoType:2; // Information type (INFO_TYPE_XXX)
        } RspEvt; // Response event
    } ParameterData;    
} RX_FCE_INFO_STRUC, *PRX_FCE_INFO_STRUC;

//
// Use READ_RX_FCE_INFO_XXX get and set fields of RX_FCE_INFO_STRUC
//
#define READ_RX_FCE_INFO_INFO_TYPE(_pAd, pRxFceInfo) \
        ((pRxFceInfo)->ParameterData.RspEvt.InfoType)
        
#define READ_RX_FCE_INFO_UDP_ERR(_pAd, pRxFceInfo) \
        ((pRxFceInfo)->ParameterData.Dot11PktInfo.UDP_ERR)

#define READ_RX_FCE_INFO_TCP_ERR(_pAd, pRxFceInfo) \
        ((pRxFceInfo)->ParameterData.Dot11PktInfo.TCP_ERR)

#define READ_RX_FCE_INFO_IP_ERR(_pAd, pRxFceInfo) \
        ((pRxFceInfo)->ParameterData.Dot11PktInfo.IP_ERR)

#define READ_RX_FCE_INFO_L3L4_DONE(_pAd, pRxFceInfo) \
        ((pRxFceInfo)->ParameterData.Dot11PktInfo.L3L4_DONE)

#define READ_RX_FCE_INFO_PKT_LENGTH(_pAd, pRxFceInfo) \
        ((pRxFceInfo)->ParameterData.RspEvt.PktLength)

#define READ_RX_FCE_INFO_SELF_GEN_RSP_EVT(_pAd, pRxFceInfo) \
        ((pRxFceInfo)->ParameterData.RspEvt.SelfGenRspEvt)

#define READ_RX_FCE_INFO_CMD_SEQ(_pAd, pRxFceInfo) \
        ((pRxFceInfo)->ParameterData.RspEvt.CmdSeq)

#define READ_RX_FCE_INFO_RSP_EVT_TYPE(_pAd, pRxFceInfo) \
        ((pRxFceInfo)->ParameterData.RspEvt.RspEvtType)

#define READ_RX_FCE_INFO_SPORT(_pAd, pRxFceInfo) \
        ((pRxFceInfo)->ParameterData.RspEvt.SPort)


//
// TX_INFO_STRUC (802.11 packet or MCU command)
//
//  1. Do not access the field directly
//  2. Use WRITE_TXD_XXX set the fields of TX_INFO_STRUC
//
/*
typedef struct _TX_INFO_STRUC
{
    union
    {
        struct // 802.11 packet
        {
            ULONG       PacketLength:16; // Packet length
            ULONG       NextVld:1; // Info DMA this frame is not the last frame in the current Tx queue
            ULONG       TxBurst:1; // Force DMA to transmit frame from current selected endpoint
            ULONG       Reserved1:1; // Reserved
            ULONG       Dot11Packet:1; // 1: 802.11 packet, 0: 802.3 packet
            ULONG       TSO:1; // TCP segmentation offload
            ULONG       CSO:1; // Checksum offload
            ULONG       Reserved2:2; // Reserved

            //
            // With IV
            //  0: Add HW-generated IV (IV/EIV register) in Tx frame
            //  1: Add SW-generated IV (TXWI.IV+TXWI.EIV) in Tx frame
            //
            ULONG       WIV:1;

            ULONG       QSEL:2; // Packet buffer queue selection
            //
            // This packet will be forwarded through which FCE port.
            // 3'b000:  WLAN port
            // 3'b001:  CPU Rx port
            // 3'b010:  CPU Tx port
            // 3'b011:  Host port (PCIe)
            // 3'b100:  Virtual CPU Rx port
            // 3'b101:  Virtual CPU Tx port
            // 3'b110:  Discard
            //
            ULONG       Port:3;
            ULONG   InfoType:2;
        } Dot11Pkt; // 802.11 packet

        struct // Packet command
        {
            ULONG   PktCmdParametersLength:16; // Length of parameters
            ULONG   PktCmdSeq:4; // Packet command sequence (NO_PKT_CMD_RSP_EVENT or sequence)
            ULONG   PktCmdType:7; // Packet command type (PKT_CMD_TYPE_XXX)
            ULONG   DPort:3; // Which the FCE port (PORT_TYPE_XXX)
            ULONG   InfoType:2;
        } PktCmd; // Packet command
    } ParameterData;

     // Information type (INFO_TYPE_XXX)
} TX_INFO_STRUC, *PTX_INFO_STRUC;
*/

//
// TxBF configurations
//
//2 TODO: Add (check InitTxBfCtrl, TxBfOidSetETxEn, TxBfOidSetETxBfTimeout and TxBfOidSetETxBfNonCompressedFeedback)
//
#define TX_TXBF_CFG_0       0x138C
#define TX_TXBF_CFG_1       0x13A4
#define TX_TXBF_CFG_2       0x13A8
#define TX_TXBF_CFG_3       0x13AC

#define BBP_REG_BF      BBP_R163 // TxBf control
#define BBP_REG_SNR0        BBP_R160 // TxBf SNR report (stream 0)
#define BBP_REG_SNR1        BBP_R161 // TxBf SNR report (stream 1)
#define BBP_REG_SNR2        BBP_R162 // TxBf SNR report (stream 2)

//
// PHY configuration
//
// 1. Do not access the field directly
// 2. Use READ_PHY_CFG_XXX and WRITE_PHY_CFG_XXX macros to get and set fields of PHY_CFG
//
typedef union  _PHY_CFG
{
    struct // Old chips, such as 2860, 3090, 539X, 559X
    {
        ULONG   MCS:7; // MCS, MCS_XXX
        ULONG   BW:1; // Bandwidth, BW_XXX
        ULONG   ShortGI:1; // Short GI
        ULONG   STBC:2; // STBC
        ULONG   bITxBfPacket:1; // Apply the beamforming matrix of the implicit TxBf
        ULONG   bETxBfPacket:1; // Apply the beamforming matrix of the explicit TxBf
        ULONG   ldpc:1; // 
        ULONG   Mode:2; // Phy mode, MODE_XXX
        ULONG   Reserved2:16; // Reserved
    } Default;

    struct // New chips, such as RT6X9X
    {
        ULONG   MCS:7; // MCS, MCS_XXX
        ULONG   BW:3; // Bandwidth, BW_XXX
        ULONG   ShortGI:1; // Short GI
        ULONG   STBC:1; // STBC
        ULONG   bITxBfPacket:1; // Apply the beamforming matrix of the implicit TxBf
        ULONG   bETxBfPacket:1; // Apply the beamforming matrix of the explicit TxBf
        ULONG   Mode:3; // Phy mode, MODE_XXX
        ULONG   Nss:2; // Nss, NSS_XXX
        ULONG   Reserved:13; // Reserved
    } Ext;

    ULONG   DoubleWord;
} PHY_CFG, *PPHY_CFG;

// MIMO Tx parameter, ShortGI, MCS, STBC, etc.  these are fields in TXWI. Don't change this definition!!!
typedef union  _MACHTTRANSMIT_SETTING {
    struct
    {
        ULONG   MCS:7; // MCS, MCS_XXX
        ULONG   BW:1; // Bandwidth, BW_XXX
        ULONG   ShortGI:1; // Short GI
        ULONG   STBC:2; // STBC
        ULONG   Rsv1:3;
        ULONG   Mode:2; // Phy mode, MODE_XXX
        ULONG   Rsv2:16; // Reserved
    } Default;

    struct // New chips, such as RT6X9X
    {
        ULONG   MCS:7; // MCS, MCS_XXX
        ULONG   BW:3; // Bandwidth, BW_XXX
        ULONG   ShortGI:1; // Short GI
        ULONG   STBC:1; // STBC
        ULONG   Rsv1:2;
        ULONG   Mode:3; // Phy mode, MODE_XXX
        ULONG   Nss:2; // Nss, NSS_XXX
        ULONG   Rsv2:13; // Reserved
    } Ext;

    ULONG   DoubleWord;
    
} MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

//
// 802.11 MAC entry
//
typedef struct _RT_802_11_MAC_ENTRY
{
    UCHAR       Addr[MAC_ADDR_LEN];
    UCHAR       Aid;
    UCHAR       Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
    MACHTTRANSMIT_SETTING   TxRate; //2 TODO: Add VHT
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

//
// PHY mode in TxWI and RxWI
//
// Note: Reserved (5~7)
//
#define MODE_CCK            0 // CCK
#define MODE_OFDM           1 // OFDM
#define MODE_HTMIX          2 // HT mixed mode
#define MODE_HTGREENFIELD   3 // HT green field
#define MODE_VHT            4 // VHT

//
// Bandwidth
//
// Note: Reserved (3)
//
#define BW_20       0 // 20MHz
#define BW_40       1 // 40MHz
#define BW_80       2 // 80MHz
#define BW_160      3 // 160MHz
#define BW_10       4 // 10MHz
#define BW_5        5 // 5MHz
#define BW_ALL      0xFF // All BW

// define according spec., use in VHT_CAP_IE only
#define NOT_SUPPORT_ABOVE_BW_80     0 // 20MHz
#define SUPPORT_BW_160              1 // 40MHz
#define SUPPORT_BW_160_80_80        2 // 80MHz

// define according spec., use in VHT_CAP_IE only
#define MAX_AMPDU_3895_OCTETS      0
#define MAX_AMPDU_7991_OCTETS      1
#define MAX_AMPDU_11454_OCTETS     2

// define according spec., use in VHT_OP_IE only
#define VHT_BW_20_40   0
#define VHT_BW_80      1
#define VHT_BW_160     2
#define VHT_BW_80_80   3

//
// 802.11 PHY mode (PHY_11XXX)
//
typedef enum _RT_802_11_PHY_MODE
{
    PHY_11BG_MIXED = 0, 
    PHY_11B = 1, 
    PHY_11A = 2, 
    PHY_11ABG_MIXED = 3, // Dual band
    PHY_11G = 4, 
    PHY_11ABGN_MIXED = 5, // Dual band
    PHY_11N = 6, 
    PHY_11GN_MIXED = 7, // 2.4GHz
    PHY_11AN_MIXED = 8, // 5GHz
    PHY_11BGN_MIXED = 9, 
    PHY_11AGN_MIXED = 10, // Dual band
    PHY_11VHT = 11, // VHT (HT on 2.4GHz and HT/VHT on 5GHz)
    PHY_11MAX = 12
} RT_802_11_PHY_MODE;

//2 TODO: Add VHT
// This is OID setting structure. So only GF or MM as Mode. This is valid when our wirelss mode has 802.11n in use.
typedef struct 
{ 
    RT_802_11_PHY_MODE PhyMode;     // 
    UCHAR TransmitNo;
    UCHAR HtMode;       // Decide PHY mode of TxWI (HTMODE_MM, HTMODE_GF, HTMODE_11B, HTMODE_11G, HTMODE_VHT)
    UCHAR ExtOffset;    // For PhyMode != PHY_11VHT
                        //    Indicate extension channel is above or below the primary channel (EXTCHA_BELOW, EXTCHA_ABOVE)
                        // For PhyMode == PHY_11VHT, refer VhtCentralChannel
    UCHAR MCS;          // For PhyMode != PHY_11VHT: (MCS_0~MCS_32, MCS_AUTO)
                        // For PhyMode == PHY_11VHT: Bit3~0 (MCS_0~MCS_9), BIT7~4 (NSS_0, NSS_1, NSS_2)
    UCHAR BW;           // (BW_20, BW_40, BW_80, BW_160, BW_10)
    UCHAR STBC;         // (STBC_NONE, STBC_USE)
    UCHAR SHORTGI;      // (GI_800, GI_400, GI_BOTH)
    UCHAR CentralChannelSeg1;    // For PhyMode == PHY_11VHT
                                // Indicate CentralChannel Index(42, 58, 106, 122, 138, 155)
                                // for CentralChannelSeg2, we are considering to use ExtOffset, more discussions are need.
} OID_SET_HT_PHYMODE, *POID_SET_HT_PHYMODE;

//
// PHY mode (HTMODE_XXX)
//
#define HTMODE_MM   0 // HT mixed mode
#define HTMODE_GF   1 // HT green field
#define HTMODE_11B  2 // 802.11b
#define HTMODE_11G  3 // 802.11g
#define HTMODE_VHT  4 // 802.11ac

//
// MCS/NSS in HT mode (for TXWI and RXWI)
//
// Note: 
//  MCS[6:3] = NSS (0~1)
//  MCS[2:0] = MCS (0~7)
//
#define HT_MCS_0            (0x00)
#define HT_MCS_1            (0x01)
#define HT_MCS_2            (0x02)
#define HT_MCS_3            (0x03)
#define HT_MCS_4            (0x04)
#define HT_MCS_5            (0x05)
#define HT_MCS_6            (0x06)
#define HT_MCS_7            (0x07)
#define HT_MCS_8            (0x08)
#define HT_MCS_9            (0x09)
#define HT_MCS_10       (0x0A)
#define HT_MCS_11       (0x0B)
#define HT_MCS_12       (0x0C)
#define HT_MCS_13       (0x0D)
#define HT_MCS_14       (0x0E)
#define HT_MCS_15       (0x0F)

//
// eFuse architecture version (eFuseArchitectureVersion_Xxx)
//
typedef enum _eFuseArchitectureVersion
{
    eFuseArchitectureVersion_First = 0, 
    eFuseArchitectureVersion_Old = 1, 
    eFuseArchitectureVersion_EfuseSizeReduction = 2, // e-fuse size reduction
    eFuseArchitectureVersion_Last = 2
} eFuseArchitectureVersion, *PeFuseArchitectureVersion;

//
// RF architecture version (RfArchitectureVersion_Xxx)
//
typedef enum _RfArchitectureVersion
{
    RfArchitectureVersion_First = 0, 
    RfArchitectureVersion_Old = 1, 

    //
    // TC600X
    //
    // 1. Multiple band selection
    //
    RfArchitectureVersion_MultipleBandSelectionEnabled = 2, 
    RfArchitectureVersion_Last = 2
} RfArchitectureVersion, *PRfArchitectureVersion;

//
// BB architecture version (BbArchitectureVersion_Xxx)
//
typedef enum _BbArchitectureVersion
{
    BbArchitectureVersion_First = 0, 
    BbArchitectureVersion_Old = 1, 

    //
    // RT6X9X
    //
    // 1. Four bytes BB register read/write
    // 2. Refactor BB registers
    //
    BbArchitectureVersion_FourBytesReadWriteEnabled = 2, 
    BbArchitectureVersion_Last = 2
} BbArchitectureVersion, *PBbArchitectureVersion;

//
// MAC architecture version (MacArchitectureVersion_Xxx)
//
typedef enum _MacArchitectureVersion
{
    MacArchitectureVersion_First = 0, 
    MacArchitectureVersion_Old = 1, 

    //
    // RT6X9X
    //  1. New TXWI/RXWI structures
    //  2. New order of RXD/RX_FCE_INFO/RXINFO/RXWI
    //  3. New register address mapping
    //
    MacArchitectureVersion_MacRegRemappingWithAndesEnabled = 2, // RT6X9X
    MacArchitectureVersion_Last = 2
} MacArchitectureVersion, *PMacArchitectureVersion;

//
// HW architecture version
//
typedef union _HW_ARCHITECTURE_VERSION
{
    struct
    {
        ULONG   MacVersion:8; // MacArchitectureVersion_Xxx
        ULONG   BbVersion:8; // BbArchitectureVersion_Xxx
        ULONG   RfVersion:8; // RfArchitectureVersion_Xxx
        ULONG   eFuseVersion:8;// eFuseArchitectureVersion_Xxx
    };

    ULONG       Word;
} HW_ARCHITECTURE_VERSION, *PHW_ARCHITECTURE_VERSION;

//
// Internal/external PA configurations in EEPROM
//
#define EEPROM_PA_CFG (0x35)

//
// Internal/external PA configurations
//
#define PA_CFG_2G_EXT_PA_AND_5G_EXT_PA(_pAd) (_pPort->CommonCfg.PACfg == 0) // 2.4GHz: external PA; 5GHz: external PA
#define PA_CFG_2G_INT_PA_AND_5G_EXT_PA(_pAd) (_pPort->CommonCfg.PACfg == 1) // 2.4GHz: internal PA; 5GHz: external PA
#define PA_CFG_2G_EXT_PA_AND_5G_INT_PA(_pAd) (_pPort->CommonCfg.PACfg == 2) // 2.4GHz: external PA; 5GHz: internal PA
#define PA_CFG_2G_INT_PA_AND_5G_INT_PA(_pAd) (_pPort->CommonCfg.PACfg == 3) // 2.4GHz: internal PA; 5GHz: internal PA

//
// UI rate display indication (e.g., 3090, 53XX, 55XX, 35XX)
//
typedef union _UI_RATE_DISPLAY_INDICATION
{
    struct
    {
        USHORT MCS:7; // MCS_XXX
        USHORT BW:1; // BW_XXX
        USHORT ShortGI:1; // GI_XXX
        USHORT STBC:2; // STBC_XXX
        USHORT bITxBfPacket:1; // 1/0
        USHORT bETxBfPacket:1; // 1/0
        USHORT Reserved:1;
        USHORT Mode:2; // MODE_XXX
    } field;

    USHORT word;
} UI_RATE_DISPLAY_INDICATION, *PUI_RATE_DISPLAY_INDICATION;

//
// UI rate display indication (e.g., RT6X9X)
//
typedef union _UI_RATE_DISPLAY_INDICATION_EXT
{
    struct
    {
        ULONG   MCS:7; // MCS, MCS_XXX
        ULONG   BW:3; // Bandwidth, BW_XXX
        ULONG   ShortGI:1; // Short GI
        ULONG   STBC:1; // STBC
        ULONG   bITxBfPacket:1; // Apply the beamforming matrix of the implicit TxBf
        ULONG   bETxBfPacket:1; // Apply the beamforming matrix of the explicit TxBf
        ULONG   Mode:3; // Phy mode, MODE_XXX
        ULONG   Nss:2; // Nss, NSS_XXX
        ULONG   Reserved:13; // Reserved
    } field;

    ULONG   DoubleWord;
} UI_RATE_DISPLAY_INDICATION_EXT, *PUI_RATE_DISPLAY_INDICATION_EXT;

//
// MCUCTL registers: Boot mode
//
#define BOOT_MODE       (0x200)

//
// MCUCTL registers: CPU control
//
#define CPU_CTL         (0x204)

//
// MCUCTL registers: Reset control
//
#define RESET_CTRL      (0x20C)

//
// MCUCTL registers: INT_LEVEL
//
#define INT_LEVEL       (0x218)

//
// MCUCTL registers: COM_REG0
//
#define COM_REG0        (0x230)

//
// MCUCTL registers: SEMAPHORE_00
//
// Note that SEMAPHORE_00[0] is W1S, RC (Write 1, Read clean)
#define SEMAPHORE_00    (0x2B0)
#define SEMAPHORE_01    (0x2B4) // RF read/write protection between Andes firmware and Wi-Fi driver


//
// The header format of Firmware bin file
//
typedef struct  _FW_BIN_HEADER_STRUC    {
        ULONG       IlmLen;
        ULONG       DlmLen;
        ULONG       BtFwVersion;
        ULONG       WiFiFwVersion;
        ULONGLONG   TimeStampLowPart;   // ASCII code
        ULONG       TimeStampHighPart;  // ASCII code   
        ULONG       Reserved;
}   FW_BIN_HEADER_STRUC, *PFW_BIN_HEADER_STRUC;

#define FIRMWARE_HEADER_LENGTH sizeof(FW_BIN_HEADER_STRUC)
#define IVECTOR_LENGTH 0x40
#define ILMFW_START_ADDRESS IVECTOR_LENGTH
#define DLMFW_START_ADDRESS (IS_TC6008(G_pAdapter)?(0x2000):(0x80000))

//
// BB blocks (BB_BLOCK_XXX)
//
//
#define BB_BLOCK_BASE               (0x2000) // Base address
#define BB_BLOCK_CORE               (BB_BLOCK_BASE + 0x0000) // Core
#define BB_BLOCK_INBAND_IF          (BB_BLOCK_BASE + 0x0100) // In-band interface
#define BB_BLOCK_AGC                (BB_BLOCK_BASE + 0x0300) // PD/AGC/SYNC
#define BB_BLOCK_TX_CCK             (BB_BLOCK_BASE + 0x0400) // CCK transmitter
#define BB_BLOCK_RX_CCK             (BB_BLOCK_BASE + 0x0500) // CCK receiver
#define BB_BLOCK_TX_OFDM            (BB_BLOCK_BASE + 0x0600) // OFDM transmitter
#define BB_BLOCK_TX_BACKEND         (BB_BLOCK_BASE + 0x0700) // Tx backend
#define BB_BLOCK_RX_FRONEND         (BB_BLOCK_BASE + 0x0800) // Rx frontend
#define BB_BLOCK_RX_OFDM            (BB_BLOCK_BASE + 0x0900) // OFDM receiver
#define BB_BLOCK_DFS                (BB_BLOCK_BASE + 0x0A00) // DFS radar detection engine
#define BB_BLOCK_TONE_RADAR         (BB_BLOCK_BASE + 0x0B00) // Tone radar detection engine
#define BB_BLOCK_CAL                (BB_BLOCK_BASE + 0x0C00) // RF/analog calibrations
#define BB_BLOCK_DATASCOPE          (BB_BLOCK_BASE + 0x0E00) // Datascope
#define BB_BLOCK_TXBF_PROFILE_MGR   (BB_BLOCK_BASE + 0x0F00) // TxBF profile manager

//
// BB offset (BB_OFFSET_RXXX)
//
//2 TODO: Check final BB spec
//
#define BB_OFFSET_R0        (0x000)
#define BB_OFFSET_R1        (0x004)
#define BB_OFFSET_R2        (0x008)
#define BB_OFFSET_R3        (0x00C)
#define BB_OFFSET_R4        (0x010)
#define BB_OFFSET_R5        (0x014)
#define BB_OFFSET_R6        (0x018)
#define BB_OFFSET_R7        (0x01C)
#define BB_OFFSET_R8        (0x020)
#define BB_OFFSET_R9        (0x024)
#define BB_OFFSET_R10       (0x028)
#define BB_OFFSET_R11       (0x02C)
#define BB_OFFSET_R12       (0x030)
#define BB_OFFSET_R13       (0x034)
#define BB_OFFSET_R14       (0x038)
#define BB_OFFSET_R15       (0x03C)
#define BB_OFFSET_R16       (0x040)
#define BB_OFFSET_R17       (0x044)
#define BB_OFFSET_R18       (0x048)
#define BB_OFFSET_R19       (0x04C)
#define BB_OFFSET_R20       (0x050)
#define BB_OFFSET_R21       (0x054)
#define BB_OFFSET_R22       (0x058)
#define BB_OFFSET_R23       (0x05C)
#define BB_OFFSET_R24       (0x060)
#define BB_OFFSET_R25       (0x064)
#define BB_OFFSET_R26       (0x068)
#define BB_OFFSET_R27       (0x06C)
#define BB_OFFSET_R28       (0x070)
#define BB_OFFSET_R29       (0x074)
#define BB_OFFSET_R30       (0x078)
#define BB_OFFSET_R31       (0x07C)
#define BB_OFFSET_R32       (0x080)
#define BB_OFFSET_R33       (0x084)
#define BB_OFFSET_R34       (0x088)
#define BB_OFFSET_R35       (0x08C)
#define BB_OFFSET_R36       (0x090)
#define BB_OFFSET_R37       (0x094)
#define BB_OFFSET_R38       (0x098)
#define BB_OFFSET_R39       (0x09C)
#define BB_OFFSET_R40       (0x0A0)
#define BB_OFFSET_R41       (0x0A4)
#define BB_OFFSET_R42       (0x0A8)
#define BB_OFFSET_R43       (0x0AC)
#define BB_OFFSET_R44       (0x0B0)
#define BB_OFFSET_R45       (0x0B4)
#define BB_OFFSET_R46       (0x0B8)
#define BB_OFFSET_R47       (0x0BC)
#define BB_OFFSET_R48       (0x0C0)
#define BB_OFFSET_R49       (0x0C4)
#define BB_OFFSET_R50       (0x0C8)
#define BB_OFFSET_R51       (0x0CC)
#define BB_OFFSET_R52       (0x0D0)
#define BB_OFFSET_R53       (0x0D4)
#define BB_OFFSET_R54       (0x0D8)
#define BB_OFFSET_R55       (0x0DC)
#define BB_OFFSET_R56       (0x0E0)
#define BB_OFFSET_R57       (0x0E4)
#define BB_OFFSET_R58       (0x0E8)
#define BB_OFFSET_R59       (0x0EC)
#define BB_OFFSET_R60       (0x0F0)
#define BB_OFFSET_R61       (0x0F4)
#define BB_OFFSET_R62       (0x0F8)
#define BB_OFFSET_R63       (0x0FC)
#define BB_OFFSET_R64       (0x100)
#define BB_OFFSET_R65       (0x104)
#define BB_OFFSET_R66       (0x108)
#define BB_OFFSET_R67       (0x10C)
#define BB_OFFSET_R68       (0x110)
#define BB_OFFSET_R69       (0x114)
#define BB_OFFSET_R70       (0x118)

//
// Bit definitions
//
#define BIT31   0x80000000
#define BIT30   0x40000000
#define BIT29   0x20000000
#define BIT28   0x10000000
#define BIT27   0x08000000
#define BIT26   0x04000000
#define BIT25   0x02000000
#define BIT24   0x01000000
#define BIT23   0x00800000
#define BIT22   0x00400000
#define BIT21   0x00200000
#define BIT20   0x00100000
#define BIT19   0x00080000
#define BIT18   0x00040000
#define BIT17   0x00020000
#define BIT16   0x00010000
#define BIT15   0x00008000
#define BIT14   0x00004000
#define BIT13   0x00002000
#define BIT12   0x00001000
#define BIT11   0x00000800
#define BIT10   0x00000400
#define BIT9    0x00000200
#define BIT8    0x00000100
#define BIT7    0x00000080
#define BIT6    0x00000040
#define BIT5    0x00000020
#define BIT4    0x00000010
#define BIT3    0x00000008
#define BIT2    0x00000004
#define BIT1    0x00000002
#define BIT0    0x00000001

//
// Write the desired bit values
//
// Usage: 
//  ULONG BbReg = 0;
//  BB_BITWISE_WRITE(BbReg, (BIT3 + BIT4), (BIT4));
//
//  ==> 1. Clear bit 3 and 4
//  ==> 2. Set bit 4
//  ==> 3. Results: 32b'0000 0000 0000 0000 0000 0000 0001 0000
//
//
#define BB_BITWISE_WRITE(_BbReg, _BitLocation, _BitValue) \
    (_BbReg = (((_BbReg) & ~(_BitLocation)) | (_BitValue)))

//
// Read the desired bits (no shift)
//
// Usage: 
//  ULONG BbReg = 0x123455F3;
//  BB_BITMASK_READ(BbReg, (BIT3 + BIT4));
//
//  ==> 1. Return bit 3 and 4, and other bits are set to zero
//  ==> 2. Results: 32b'0000 0000 0000 0000 0000 0000 0001 0000
//
#define BB_BITMASK_READ(_BbReg, _BitLocation) \
    ((_BbReg) & (_BitLocation))

//
// Get the first byte of BB value
//
// Usage: 
//  ULONG BbReg = 0x12345678;
//  BB_GET_BYTE0(BbReg);
//
//  ==> Return 0x78
//
#define BB_GET_BYTE0(_BbReg) \
    ((_BbReg) & 0x000000FF)

//
// Get the second byte of BB value
//
// Usage: 
//  ULONG BbReg = 0x12345678;
//  BB_GET_BYTE1(BbReg);
//
//  ==> Return 0x56
//
//
#define BB_GET_BYTE1(_BbReg) \
    (((_BbReg) & 0x0000FF00) >> 8)

//
// Get the third byte of BB value
//
// Usage: 
//  ULONG BbReg = 0x12345678;
//  BB_GET_BYTE2(BbReg);
//
//  ==> Return 0x34
//
//
#define BB_GET_BYTE2(_BbReg) \
    (((_BbReg) & 0x00FF0000) >> 16)

//
// Get the fourth byte of BB value
//
// Usage: 
//  ULONG BbReg = 0x12345678;
//  BB_GET_BYTE3(BbReg);
//
//  ==> Return 0x12
//
//
#define BB_GET_BYTE3(_BbReg) \
    (((_BbReg) & 0xFF000000) >> 24)

//
// Replace the first byte of BB value
//
// Usage: 
//  ULONG BbReg = 0x12345678;
//  UCHAR BbValueByte = 0xAB;
//  BB_SET_BYTE0(BbReg, BbValueByte);
//
//  ==> Return 0x123456AB
//
#define BB_SET_BYTE0(_BbReg, _BbValueByte) \
    ((_BbReg) = (((_BbReg) & ~0x000000FF) | (_BbValueByte)))

//
// Replace the second byte of BB value
//
// Usage: 
//  ULONG BbReg = 0x12345678;
//  UCHAR BbValueByte = 0xAB;
//  BB_SET_BYTE1(BbReg, BbValueByte);
//
//  ==> Return 0x1234AB78
//
#define BB_SET_BYTE1(_BbReg, _BbValueByte) \
    ((_BbReg) = (((_BbReg) & ~0x0000FF00) | (_BbValueByte << 8)))

//
// Replace the third byte of BB value
//
// Usage: 
//  ULONG BbReg = 0x12345678;
//  UCHAR BbValueByte = 0xAB;
//  BB_SET_BYTE2(BbReg, BbValueByte);
//
//  ==> Return 0x12AB5678
//
#define BB_SET_BYTE2(_BbReg, _BbValueByte) \
    ((_BbReg) = (((_BbReg) & ~0x00FF0000) | (_BbValueByte << 16)))

//
// Replace the fourth byte of BB value
//
// Usage: 
//  ULONG BbReg = 0x12345678;
//  UCHAR BbValueByte = 0xAB;
//  BB_SET_BYTE3(BbReg, BbValueByte);
//
//  ==> Return 0xAB345678
//
#define BB_SET_BYTE3(_BbReg, _BbValueByte) \
    ((_BbReg) = (((_BbReg) & ~0xFF000000) | (_BbValueByte << 24)))

//
// Read BB register
//
// Usage: 
//  ULONG BbValue = 0;
//  BB_READ(pAd, (BB_BLOCK_XXX + BB_OFFSET_RXXX), &BbValue);
//
#define BB_READ32(_pAd, _BbBlock, _BbAddress, _BbValue) \
{ \
    RTUSBReadMACRegister(_pAd, _BbBlock+_BbAddress , _BbValue); \
}

//
// Write BB register
//
// Usage: 
//  ULONG BbValue = 0;
//  BB_WRITE(pAd, (BB_BLOCK_XXX + BB_OFFSET_RXXX), BbValue);
//
#define BB_WRITE32(_pAd, _BbBlock, _BbAddress, _BbValue) \
{ \
    RTUSBWriteMACRegister(_pAd, _BbBlock+_BbAddress, _BbValue); \
}

//
// Write the desired bit values
//
// Usage: 
//  ULONG MacReg = 0;
//  MAC_BITWISE_WRITE(MacReg, (BIT3 + BIT4), (BIT4));
//
//  ==> 1. Clear bit 3 and 4
//  ==> 2. Set bit 4
//  ==> 3. Results: 32b'0000 0000 0000 0000 0000 0000 0001 0000
//
//
#define MAC_BITWISE_WRITE(_MacReg, _BitLocation, _BitValue) \
    (_MacReg = (((_MacReg) & ~(_BitLocation)) | (_BitValue)))

//
// Read the desired bits (no shift)
//
// Usage: 
//  ULONG MacReg = 0x123455F3;
//  MAC_BITMASK_READ(BbReg, (BIT3 + BIT4));
//
//  ==> 1. Return bit 3 and 4, and other bits are set to zero
//  ==> 2. Results: 32b'0000 0000 0000 0000 0000 0000 0001 0000
//
#define MAC_BITMASK_READ(_MacReg, _BitLocation) \
    ((_MacReg) & (_BitLocation))

//
// Get the first byte of MAC value
//
// Usage: 
//  ULONG MacReg = 0x12345678;
//  MAC_GET_BYTE0(MacReg);
//
//  ==> Return 0x78
//
#define MAC_GET_BYTE0(_MacReg) \
    ((_MacReg) & 0x000000FF)

//
// Get the second byte of MAC value
//
// Usage: 
//  ULONG MacReg = 0x12345678;
//  MAC_GET_BYTE1(MacReg);
//
//  ==> Return 0x56
//
//
#define MAC_GET_BYTE1(_MacReg) \
    (((_MacReg) & 0x0000FF00) >> 8)

//
// Get the third byte of MAC value
//
// Usage: 
//  ULONG MacReg = 0x12345678;
//  MAC_GET_BYTE2(MacReg);
//
//  ==> Return 0x34
//
//
#define MAC_GET_BYTE2(_MacReg) \
    (((_MacReg) & 0x00FF0000) >> 16)

//
// Get the fourth byte of MAC value
//
// Usage: 
//  ULONG MacReg = 0x12345678;
//  MAC_GET_BYTE3(MacReg);
//
//  ==> Return 0x12
//
//
#define MAC_GET_BYTE3(_MacReg) \
    (((_MacReg) & 0xFF000000) >> 24)

//
// Replace the first byte of MAC value
//
// Usage: 
//  ULONG MacReg = 0x12345678;
//  UCHAR MacValueByte = 0xAB;
//  MAC_SET_BYTE0(MacReg, MacValueByte);
//
//  ==> Return 0x123456AB
//
#define MAC_SET_BYTE0(_MacReg, _MacValueByte) \
    ((_MacReg) = (((_MacReg) & ~0x000000FF) | (_MacValueByte)))

//
// Replace the second byte of MAC value
//
// Usage: 
//  ULONG MacReg = 0x12345678;
//  UCHAR MacValueByte = 0xAB;
//  MAC_SET_BYTE1(MacReg, MacValueByte);
//
//  ==> Return 0x1234AB78
//
#define MAC_SET_BYTE1(_MacReg, _MacValueByte) \
    ((_MacReg) = (((_MacReg) & ~0x0000FF00) | (_MacValueByte << 8)))

//
// Replace the third byte of MAC value
//
// Usage: 
//  ULONG MacReg = 0x12345678;
//  UCHAR MacValueByte = 0xAB;
//  MAC_SET_BYTE2(MacReg, MacValueByte);
//
//  ==> Return 0x12AB5678
//
#define MAC_SET_BYTE2(_MacReg, _MacValueByte) \
    ((_MacReg) = (((_MacReg) & ~0x00FF0000) | (_MacValueByte << 16)))

//
// Replace the fourth byte of MAC value
//
// Usage: 
//  ULONG MacReg = 0x12345678;
//  UCHAR MacValueByte = 0xAB;
//  MAC_SET_BYTE3(MacReg, MacValueByte);
//
//  ==> Return 0xAB345678
//
#define MAC_SET_BYTE3(_MacReg, _MacValueByte) \
    ((_MacReg) = (((_MacReg) & ~0xFF000000) | (_MacValueByte << 24)))

//
// Write the desired bit values
//
// Usage: 
//  ULONG RfReg = 0;
//  RF_BITWISE_WRITE(RfReg, (BIT3 + BIT4), (BIT4));
//
//  ==> 1. Clear bit 3 and 4
//  ==> 2. Set bit 4
//  ==> 3. Results: 8b'0000 0000
//
//
#define RF_BITWISE_WRITE(_RfReg, _BitLocation, _BitValue) \
    (_RfReg = (((_RfReg) & ~(_BitLocation)) | (_BitValue)))

//
// Read the desired bits (no shift)
//
// Usage: 
//  ULONG RfReg = 0x55F3;
//  RF_BITMASK_READ(RfReg, (BIT3 + BIT4));
//
//  ==> 1. Return bit 3 and 4, and other bits are set to zero
//  ==> 2. Results: 8b'0000 0000 0001 0000
//
#define RF_BITMASK_READ(_RfReg, _BitLocation) \
    ((_RfReg) & (_BitLocation))

extern USHORT CckOfdmHtRateTable[];
extern USHORT VhtRateTable[];

//
// The size, in bytes, of an entry in the rate switch table
//
#define SIZE_OF_RATE_TABLE_ENTRY            (6)

//
// Tx rate switch
//
typedef struct  _RTMP_TX_RATE_SWITCH
{
    UCHAR ItemNo;
    
    UCHAR STBC:1; // STBC_XXX
    UCHAR ShortGI:1; // GI_XXX
    UCHAR BW:2; // BW_XXX
    UCHAR Mode:3; // MODE_XXX
    UCHAR Reserved1:1; // Reserved
        
    UCHAR CurrMCS;
    UCHAR TrainUp;
    UCHAR TrainDown;

    UCHAR Nss:2; // NSS_XXX (VHT only)
    UCHAR Reserved2:6; // Reserved
} RTMP_TX_RATE_SWITCH, *PRTMP_TX_RATE_SWITCH;


//
// The size, in bytes, of an AGS entry in the rate switch table
//
#define SIZE_OF_AGS_RATE_TABLE_ENTRY    (10)

//
// Tx rate switch for AGS
//
typedef struct  _RTMP_TX_RATE_SWITCH_AGS
{
    UCHAR   ItemNo;
    
    UCHAR STBC:1; // STBC_XXX
    UCHAR ShortGI:1; // GI_XXX
    UCHAR BW:2; // BW_XXX
    UCHAR Mode:3; // MODE_XXX
    UCHAR Reserved1:1; // Reserved
    
    UCHAR Nss:2; // NSS_XXX (VHT only)
    UCHAR Reserved2:6; // Reserved

    UCHAR   CurrMCS;
    UCHAR   TrainUp;
    UCHAR   TrainDown;
    UCHAR   downMcs;
    UCHAR   upMcs3;
    UCHAR   upMcs2;
    UCHAR   upMcs1;
} RTMP_TX_RATE_SWITCH_AGS, *PRTMP_TX_RATE_SWITCH_AGS;

//
// AGS VHT 1x1/2x2 rate tables
//
extern UCHAR Ags1x1VhtRateTable[];
extern UCHAR Ags2x2VhtRateTable[];
extern UCHAR Ags1x1Vht256QAMRateTable[];
extern UCHAR AgsRssiOffsetTable[3][4];

//
// Check the entry attributes (HT/MCS) of the AGS HT rate table
//
#define AGS_CCK_MCS0(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable)) && \
                              (_pAgsHtRateTableEntry->Mode == MODE_CCK) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_0))

#define AGS_CCK_MCS1(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable)) && \
                              (_pAgsHtRateTableEntry->Mode == MODE_CCK) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_1))

#define AGS_CCK_MCS2(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable)) && \
                              (_pAgsHtRateTableEntry->Mode == MODE_CCK) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_2))

#define AGS_HT_MCS0(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable5G)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_0))

#define AGS_HT_MCS1(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable5G)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_1))

#define AGS_HT_MCS2(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable5G)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_2))

#define AGS_HT_MCS3(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable5G)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_3))

#define AGS_HT_MCS4(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable5G)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_4))

#define AGS_HT_MCS5(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable5G)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_5))

#define AGS_HT_MCS6(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable5G)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_6))

#define AGS_HT_MCS7(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable) || (_pAgsHtRateTable == AGS1x1HTRateTable5G)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_7))

#define AGS_HT_MCS8(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_8))

#define AGS_HT_MCS9(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_9))

#define AGS_HT_MCS10(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_10))

#define AGS_HT_MCS11(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_11))

#define AGS_HT_MCS12(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_12))

#define AGS_HT_MCS13(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_13))

#define AGS_HT_MCS14(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_14))

#define AGS_HT_MCS15(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            (((_pAgsHtRateTable == AGS3x3HTRateTable) || (_pAgsHtRateTable == AGS2x2HTRateTable)) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_15))

#define AGS_HT_MCS16(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            ((_pAgsHtRateTable == AGS3x3HTRateTable) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_16))

#define AGS_HT_MCS17(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            ((_pAgsHtRateTable == AGS3x3HTRateTable) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_17))

#define AGS_HT_MCS18(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            ((_pAgsHtRateTable == AGS3x3HTRateTable) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_18))

#define AGS_HT_MCS19(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            ((_pAgsHtRateTable == AGS3x3HTRateTable) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_19))

#define AGS_HT_MCS20(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            ((_pAgsHtRateTable == AGS3x3HTRateTable) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_20))

#define AGS_HT_MCS21(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            ((_pAgsHtRateTable == AGS3x3HTRateTable) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_21))

#define AGS_HT_MCS22(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            ((_pAgsHtRateTable == AGS3x3HTRateTable) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_22))

#define AGS_HT_MCS23(_pAgsHtRateTable, _pAgsHtRateTableEntry) \
                            ((_pAgsHtRateTable == AGS3x3HTRateTable) && \
                              ((_pAgsHtRateTableEntry->Mode == MODE_HTGREENFIELD) || (_pAgsHtRateTableEntry->Mode == MODE_HTMIX)) && \
                              (_pAgsHtRateTableEntry->CurrMCS == MCS_23))

//
// Check the entry attributes (VHT/Nss/MCS) of the AGS VHT rate table
//
#define AGS_VHT_NSS0_MCS0(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             (((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) || (IS_VHT_1X1_AGS_RATE_TABLE(_pAgsVhtRateTable))) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_0) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_0))

#define AGS_VHT_NSS0_MCS1(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             (((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) || (IS_VHT_1X1_AGS_RATE_TABLE(_pAgsVhtRateTable))) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_0) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_1))

#define AGS_VHT_NSS0_MCS2(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             (((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) || (IS_VHT_1X1_AGS_RATE_TABLE(_pAgsVhtRateTable))) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_0) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_2))

#define AGS_VHT_NSS0_MCS3(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             (((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) || (IS_VHT_1X1_AGS_RATE_TABLE(_pAgsVhtRateTable))) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_0) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_3))

#define AGS_VHT_NSS0_MCS4(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             (((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) || (IS_VHT_1X1_AGS_RATE_TABLE(_pAgsVhtRateTable))) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_0) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_4))

#define AGS_VHT_NSS0_MCS5(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             (((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) || (IS_VHT_1X1_AGS_RATE_TABLE(_pAgsVhtRateTable))) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_0) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_5))

#define AGS_VHT_NSS0_MCS6(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             (((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) || (IS_VHT_1X1_AGS_RATE_TABLE(_pAgsVhtRateTable))) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_0) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_6))

#define AGS_VHT_NSS0_MCS7(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             (((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) || (IS_VHT_1X1_AGS_RATE_TABLE(_pAgsVhtRateTable))) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_0) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_7))

#define AGS_VHT_NSS0_MCS8(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             (((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) || (IS_VHT_1X1_AGS_RATE_TABLE(_pAgsVhtRateTable))) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_0) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_8))

#define AGS_VHT_NSS0_MCS9(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             (((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) || (IS_VHT_1X1_AGS_RATE_TABLE(_pAgsVhtRateTable))) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_0) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_9))

#define AGS_VHT_NSS1_MCS0(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             ((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_1) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_0))

#define AGS_VHT_NSS1_MCS1(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             ((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_1) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_1))

#define AGS_VHT_NSS1_MCS2(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             ((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_1) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_2))

#define AGS_VHT_NSS1_MCS3(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             ((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_1) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_3))

#define AGS_VHT_NSS1_MCS4(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             ((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_1) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_4))

#define AGS_VHT_NSS1_MCS5(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             ((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_1) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_5))

#define AGS_VHT_NSS1_MCS6(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             ((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_1) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_6))

#define AGS_VHT_NSS1_MCS7(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             ((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_1) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_7))

//2 TODO: Not support 256QAM yet
#define AGS_VHT_NSS1_MCS8(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             ((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_1) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_8))

//2 TODO: Not support 256QAM yet
#define AGS_VHT_NSS1_MCS9(_pAgsVhtRateTable, pAgsVhtRateTableEntry) \
                             ((IS_VHT_2X2_AGS_RATE_TABLE(_pAgsVhtRateTable)) && \
                               (pAgsVhtRateTableEntry->Mode == MODE_VHT) && \
                               (pAgsVhtRateTableEntry->Nss == NSS_1) && \
                               (pAgsVhtRateTableEntry->CurrMCS == MCS_9))

//
// VHT Rx AGC VGA tuning entry
//
typedef struct _VHT_RX_AGC_VGA_TUNING_ENTRY
{
    CHAR RssiThresholdForShortDistance; // RSSI threshold for short distance
    UCHAR RxAgcVgaForShortDistance; // Rx AGC VGA value for short distance
    UCHAR RxAgcVagForLongDistance; // Rx AGC VGA value for long distance
} VHT_RX_AGC_VGA_TUNING_ENTRY, *PVHT_RX_AGC_VGA_TUNING_ENTRY;

//
// VHT Rx AGC VGA tuning control
//
typedef struct _VHT_RX_AGC_VGA_TUNING_CTRL
{
    BOOLEAN bEnableVhtRxAgcVgaTuning; // Enable VHT Rx AGC VGA tuning
    UCHAR CurrentRxAgcVgaValue; // Current Rx AGC VGA value (assume all chains use the same value)
    PVHT_RX_AGC_VGA_TUNING_ENTRY pVhtRxAgcVgaTuningEntry; // Point the desired VHT Rx AGC VGA tuning entry
} VHT_RX_AGC_VGA_TUNING_CTRL, *PVHT_RX_AGC_VGA_TUNING_CTRL;

//
// Read/write RF register (via RF_CSR_CFG)
//
#define READ_RF_REGISTER 0
#define WRITE_RF_REGISTER 1

//
// Trigger RF register read/write (via RF_CSR_CFG)
//
#define TRIGGER_RF_REGISTER_READ_WRITE 1

//
// Minimum/maximum RF bank number
//
#define MIN_RF_BANK_NUM 0
#define MAX_RF_BANK_NUM 7

//
// Minimum/maximum RF register number
//
#define MIN_RF_REG_NUM 0
#define MAX_RF_REG_NUM 127

//
// RF bank number mask
//
#define RF_BANK_NUMBER_MASK 0xF0000000

//
// RF register number mask
//
#define RF_REGISTER_NUMBER_MASK 0x0FFFFFFF

//
// RF register map
//
typedef struct _RF_REGISTER_TABLE_ENTRY
{
    UCHAR BankNum; // RF bank number (RF_BANK_NUM_XXX)
    UCHAR RegNum; // RF register number (RF_XXX)
    UCHAR Value; // RF register value
} RF_REGISTER_TABLE_ENTRY, *PRF_REGISTER_TABLE_ENTRY;

//
// VHT RF register configuration
//
#ifdef BIG_ENDIAN
typedef union _RF_CSR_CFG_VHT_STRUC
{
    struct
    {
        ULONG RfCsrKick:1; // Kick RF register read/write
        ULONG RfCsrWr:1; // 0: read; 1: write
        ULONG Reserved:12; // Reserved
        ULONG RfBankNum:3; // RF bank number (RF_BANK_NUM_XXX)
        ULONG RfRegNum:7; // RF register number (RF_RXXX)
        ULONG RfCsrData:8; // Data
    } field;

    ULONG word;
} RF_CSR_CFG_VHT_STRUC, *PRF_CSR_CFG_VHT_STRUC;
#else
typedef union _RF_CSR_CFG_VHT_STRUC
{
    struct
    {
        ULONG RfCsrData:8; // Data
        ULONG RfRegNum:7; // RF register number (RF_RXXX)
        ULONG RfBankNum:3; // RF bank number (RF_BANK_NUM_XXX)
        ULONG Reserved:12; // Reserved
        ULONG RfCsrWr:1; // 0: read; 1: write
        ULONG RfCsrKick:1; // Kick RF register read/write   
    } field;
    
    ULONG word;
} RF_CSR_CFG_VHT_STRUC, *PRF_CSR_CFG_VHT_STRUC;
#endif

//
// RF bank number (RF_BANK_NUM_XXX)
//
#define RF_BANK_NUM_0 0
#define RF_BANK_NUM_1 1
#define RF_BANK_NUM_2 2
#define RF_BANK_NUM_3 3
#define RF_BANK_NUM_4 4
#define RF_BANK_NUM_5 5
#define RF_BANK_NUM_6 6
#define RF_BANK_NUM_7 7

//
// RF register number (RF_RXXX)
//
#define RF_R00 0
#define RF_R01 1
#define RF_R02 2
#define RF_R03 3
#define RF_R04 4
#define RF_R05 5
#define RF_R06 6
#define RF_R07 7
#define RF_R08 8
#define RF_R09 9
#define RF_R10 10
#define RF_R11 11
#define RF_R12 12
#define RF_R13 13
#define RF_R14 14
#define RF_R15 15
#define RF_R16 16
#define RF_R17 17
#define RF_R18 18
#define RF_R19 19
#define RF_R20 20
#define RF_R21 21
#define RF_R22 22
#define RF_R23 23
#define RF_R24 24
#define RF_R25 25
#define RF_R26 26
#define RF_R27 27
#define RF_R28 28
#define RF_R29 29
#define RF_R30 30
#define RF_R31 31
#define RF_R32 32
#define RF_R33 33
#define RF_R34 34
#define RF_R35 35
#define RF_R36 36
#define RF_R37 37
#define RF_R38 38
#define RF_R39 39
#define RF_R40 40
#define RF_R41 41
#define RF_R42 42
#define RF_R43 43
#define RF_R44 44
#define RF_R45 45
#define RF_R46 46
#define RF_R47 47
#define RF_R48 48
#define RF_R49 49
#define RF_R50 50
#define RF_R51 51
#define RF_R52 52
#define RF_R53 53
#define RF_R54 54
#define RF_R55 55
#define RF_R56 56
#define RF_R57 57
#define RF_R58 58
#define RF_R59 59
#define RF_R60 60
#define RF_R61 61
#define RF_R62 62
#define RF_R63 63
#define RF_R64 64
#define RF_R65 65
#define RF_R66 66
#define RF_R67 67
#define RF_R68 68
#define RF_R69 69
#define RF_R70 70
#define RF_R71 71
#define RF_R72 72
#define RF_R73 73
#define RF_R74 74
#define RF_R75 75
#define RF_R76 76
#define RF_R77 77
#define RF_R78 78
#define RF_R79 79
#define RF_R80 80
#define RF_R81 81
#define RF_R82 82
#define RF_R83 83
#define RF_R84 84
#define RF_R85 85
#define RF_R86 86
#define RF_R87 87
#define RF_R88 88
#define RF_R89 89
#define RF_R90 90
#define RF_R91 91
#define RF_R92 92
#define RF_R93 93
#define RF_R94 94
#define RF_R95 95
#define RF_R96 96
#define RF_R97 97
#define RF_R98 98
#define RF_R99 99
#define RF_R100 100
#define RF_R101 101
#define RF_R102 102
#define RF_R103 103
#define RF_R104 104
#define RF_R105 105
#define RF_R106 106
#define RF_R107 107
#define RF_R108 108
#define RF_R109 109
#define RF_R110 110
#define RF_R111 111
#define RF_R112 112
#define RF_R113 113
#define RF_R114 114
#define RF_R115 115
#define RF_R116 116
#define RF_R117 117
#define RF_R118 118
#define RF_R119 119
#define RF_R120 120
#define RF_R121 121
#define RF_R122 122
#define RF_R123 123
#define RF_R124 124
#define RF_R125 125
#define RF_R126 126
#define RF_R127 127

//
// Support RF bank selection
//
#define RF_BANK_SELECTION_CAPABLE(_pAd)   (0)

//
// 6X9X BB register pair
//
typedef struct  _BBP_REG_PAIR_6X9X
{
    USHORT  BbpRegBlock;
    USHORT  BbpRegOffset;
    ULONG   Value;
} BBP_REG_PAIR_6X9X, *PBBP_REG_PAIR_6X9X;

//
// The configuration type of updating BB register (BB_UPDATE_CFG_TYPE_XXX)
//
#define BB_UPDATE_CFG_TYPE_DIRECT 0 // Update directly
#define BB_UPDATE_CFG_TYPE_CHANNEL_BOUNDING 1 // Base on channel bounding
#define BB_UPDATE_CFG_TYPE_BW 2 // Base on bandwidth
#define BB_UPDATE_CFG_TYPE_PHY_MODE 3 // Base on PHY mode
#define BB_UPDATE_CFG_TYPE_2DOT4_GHZ_ALL_BW 4 // For 2.4GHz channel frequency, all bandwidth
#define BB_UPDATE_CFG_TYPE_2DOT4_GHZ_BW20 5 // For 2.4GHz channel frequency, BW20
#define BB_UPDATE_CFG_TYPE_2DOT4_GHZ_BW40 6 // For 2.4GHz channel frequency, BW40
#define BB_UPDATE_CFG_TYPE_5_GHZ_ALL_BW 7 // For 5GHz channel frequency, all bandwidth
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW20 8 // For 5GHz channel frequency, BW20
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW40 9 // For 5GHz channel frequency, BW40
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW80 10 // For 5GHz channel frequency, BW80
#define BB_UPDATE_CFG_TYPE_2DOT4_GHZ_BW20_NORMAL_TEMPERATURE 11 // For 2.4GHz, BW20, normal temperature
#define BB_UPDATE_CFG_TYPE_2DOT4_GHZ_BW20_HIGH_TEMPERATURE 12 // For 2.4GHz, BW20, high temperature 
#define BB_UPDATE_CFG_TYPE_2DOT4_GHZ_BW20_LOW_TEMPERATURE 13 // For 2.4GHz, BW20, low temperature
#define BB_UPDATE_CFG_TYPE_2DOT4_GHZ_BW40_NORMAL_TEMPERATURE 14 // For 2.4GHz, BW40, normal temperature
#define BB_UPDATE_CFG_TYPE_2DOT4_GHZ_BW40_HIGH_TEMPERATURE 15 // For 2.4GHz, BW40, high temperature
#define BB_UPDATE_CFG_TYPE_2DOT4_GHZ_BW40_LOW_TEMPERATURE 16 // For 2.4GHz, BW40, low temperature
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW20_NORMAL_TEMPERATURE 17 // For 5GHz, BW20, normal temperature
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW20_HIGH_TEMPERATURE 18 // For 5GHz, BW20, high temperature
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW20_LOW_TEMPERATURE 19 // For 5GHz, BW20, low temperature
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW40_NORMAL_TEMPERATURE 20 // For 5GHz, BW40, normal temperature
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW40_HIGH_TEMPERATURE 21 // For 5GHz, BW40, high temperature
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW40_LOW_TEMPERATURE 22 // For 5GHz, BW40, low temperature
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW80_NORMAL_TEMPERATURE 23 // For 5GHz, BW80, normal temperature
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW80_HIGH_TEMPERATURE 24 // For 5GHz, BW80, high temperature
#define BB_UPDATE_CFG_TYPE_5_GHZ_BW80_LOW_TEMPERATURE 25 // For 5GHz, BW80, low temperature

//
// BB update register value
//
// The relationship between BB_UPDATE_CFG_TYPE_XXX and structure fields: 
//
// BB_UPDATE_CFG_TYPE_DIRECT: UpdateCfgType, BlockNum, RegOffset and Value
// BB_UPDATE_CFG_TYPE_CHANNEL_BOUNDING: UpdateCfgType, BlockNum, RegOffset, Value, StartChannel and EndChannel
// BB_UPDATE_CFG_TYPE_BW: UpdateCfgType, BlockNum, RegOffset, Value and BW
// BB_UPDATE_CFG_TYPE_PHY_MODE: UpdateCfgType, BlockNum, RegOffset, Value and PhyMode
// BB_UPDATE_CFG_TYPE_2DOT4_GHZ_ALL_BW: UpdateCfgType, BlockNum, RegOffset, Value
// BB_UPDATE_CFG_TYPE_2DOT4_GHZ_BW20: UpdateCfgType, BlockNum, RegOffset, Value
// BB_UPDATE_CFG_TYPE_2DOT4_GHZ_BW40: UpdateCfgType, BlockNum, RegOffset, Value
// BB_UPDATE_CFG_TYPE_5_GHZ_ALL_BW: UpdateCfgType, BlockNum, RegOffset, Value
// BB_UPDATE_CFG_TYPE_5_GHZ_BW20: UpdateCfgType, BlockNum, RegOffset, Value
// BB_UPDATE_CFG_TYPE_5_GHZ_BW40: UpdateCfgType, BlockNum, RegOffset, Value
// BB_UPDATE_CFG_TYPE_5_GHZ_BW80: UpdateCfgType, BlockNum, RegOffset, Value
//
typedef struct _BB_UPDATE_REGISTER_VALUE
{
    BOOLEAN (*pNicVersion)(PMP_ADAPTER); // Function pointer of NIC version (not check if NULL)
    
    UCHAR UpdateCfgType; // The configuration type of updating RF register (BB_UPDATE_CFG_TYPE_XXX)
    USHORT BlockNum; // BB block number (BB_BLOCK_XXX)
    USHORT RegOffset; // BB register offset  (BB_OFFSET_RXX)
    ULONG Value; // BB register value
    
    UCHAR StartChannel; // Start channel
    UCHAR EndChannel; // End channel
    UCHAR BW; // Bandwidth (BW_XX)
    UCHAR PhyMode; // PHY mode (MODE_XXX)
} BB_UPDATE_REGISTER_VALUE, *PBB_UPDATE_REGISTER_VALUE;

//
// The configuration type of updating RF register (RF_UPDATE_CFG_TYPE_XXX)
//
#define RF_UPDATE_CFG_TYPE_DIRECT 0 // Update directly
#define RF_UPDATE_CFG_TYPE_CHANNEL_BOUNDING 1 // Base on channel bounding
#define RF_UPDATE_CFG_TYPE_BW 2 // Base on bandwidth
#define RF_UPDATE_CFG_TYPE_PHY_MODE 3 // Base on PHY mode
#define RF_UPDATE_CFG_TYPE_2DOT4_GHZ_ALL_BW 4 // For 2.4GHz channel frequency, all bandwidth
#define RF_UPDATE_CFG_TYPE_2DOT4_GHZ_BW20 5 // For 2.4GHz channel frequency, BW20
#define RF_UPDATE_CFG_TYPE_2DOT4_GHZ_BW40 6 // For 2.4GHz channel frequency, BW40
#define RF_UPDATE_CFG_TYPE_5_GHZ_ALL_BW 7 // For 5GHz channel frequency, all bandwidth
#define RF_UPDATE_CFG_TYPE_5_GHZ_BW20 8 // For 5GHz channel frequency, BW20
#define RF_UPDATE_CFG_TYPE_5_GHZ_BW40 9 // For 5GHz channel frequency, BW40
#define RF_UPDATE_CFG_TYPE_5_GHZ_BW80 10 // For 5GHz channel frequency, BW80
#define RF_UPDATE_CFG_TYPE_CHANNEL_BOUNDING_AND_BW 11 // Base on channel bounding and bandwidth

//
// RF update register value
//
// The relationship between RF_UPDATE_CFG_TYPE_XXX and structure fields: 
//
// RF_UPDATE_CFG_TYPE_DIRECT: UpdateCfgType, BankNum, RegNum and Value
// RF_UPDATE_CFG_TYPE_CHANNEL_BOUNDING: UpdateCfgType, BankNum, RegNum, Value, StartChannel and EndChannel
// RF_UPDATE_CFG_TYPE_BW: UpdateCfgType, BankNum, RegNum, Value and BW
// RF_UPDATE_CFG_TYPE_PHY_MODE: UpdateCfgType, BankNum, RegNum, Value and PhyMode
// RF_UPDATE_CFG_TYPE_2DOT4_GHZ_ALL_BW: UpdateCfgType, BankNum, RegNum, Value
// RF_UPDATE_CFG_TYPE_2DOT4_GHZ_BW20: UpdateCfgType, BankNum, RegNum, Value
// RF_UPDATE_CFG_TYPE_2DOT4_GHZ_BW40: UpdateCfgType, BankNum, RegNum, Value
// RF_UPDATE_CFG_TYPE_5_GHZ_ALL_BW: UpdateCfgType, BankNum, RegNum, Value
// RF_UPDATE_CFG_TYPE_5_GHZ_BW20: UpdateCfgType, BankNum, RegNum, Value
// RF_UPDATE_CFG_TYPE_5_GHZ_BW40: UpdateCfgType, BankNum, RegNum, Value
// RF_UPDATE_CFG_TYPE_5_GHZ_BW80: UpdateCfgType, BankNum, RegNum, Value
// RF_UPDATE_CFG_TYPE_CHANNEL_BOUNDING_AND_BW: UpdateCfgType, BankNum, RegNum, Value, StartChannel, EndChannel and BW
//
typedef struct _RF_UPDATE_REGISTER_VALUE
{
    UCHAR UpdateCfgType; // The configuration type of updating BB register (BB_UPDATE_CFG_TYPE_XXX)
    UCHAR BankNum; // RF bank number (RF_BANK_NUM_XXX)
    UCHAR RegNum; // RF register number (RF_RXXX)
    UCHAR Value; // RF register value

    UCHAR StartChannel; // Start channel
    UCHAR EndChannel; // End channel
    UCHAR BW; // Bandwidth (BW_XX)
    UCHAR PhyMode; // PHY mode (MODE_XXX)
} RF_UPDATE_REGISTER_VALUE, *PRF_UPDATE_REGISTER_VALUE;

//
// Frequency table entry
//
typedef struct _FREQUENCY_TABLE_ENTRY
{
    UCHAR Channel; // Channel number
    
    UCHAR PLL_N_R30; // PLL N: R30<0>
    UCHAR PLL_N_R29; // PLL N: R29<7:0>
    UCHAR PLL_K_R31; // PLL K: R31<4:0>
    UCHAR NON_SIGMA_R24; // Non-sigma: R24<1:0>
    UCHAR NON_SIGMA_R31; // Non-sigma: R31<7:5>
    UCHAR DENOM_8_R32; // Denom-8: R32<4:0>
    UCHAR FRAC_COMP_R33; // Frac comp: R33
    UCHAR FRAC_COMP_R34; // Frac comp: R34
    UCHAR FRAC_COMP_R35; // Frac comp: R35
} FREQUENCY_TABLE_ENTRY, *PFREQUENCY_TABLE_ENTRY;

//
// Frequency table entry (extention)
//
typedef struct _FREQUENCY_TABLE_ENTRY_EXT
{
    BOOLEAN bAppyThisEntry; // Apply this entry for channel switch
    
    UCHAR Channel; // Channel number

    UCHAR R37;
    UCHAR R36;
    UCHAR R35;
    UCHAR R34;
    UCHAR R33;
    UCHAR R32_BIT765;
    UCHAR R32_BIT43210;
    UCHAR R31_BIT765;
    UCHAR R31_BIT43210;
    UCHAR R30_BIT7;
    UCHAR R30_BIT65432;
    UCHAR R30_BIT1;
    USHORT R30_BIT0_R29;
    UCHAR R28_BIT76;
    UCHAR R28_BIT54;
    UCHAR R28_BIT32;
    ULONG R28_BIT10_R27_R26;
    UCHAR R24_BIT10;
} FREQUENCY_TABLE_ENTRY_EXT, *PFREQUENCY_TABLE_ENTRY_EXT;


//
// VCO mode (VCO_MODE_XXX)
//
#define VCO_MODE_OPEN_LOOP_CALIBRATION 0 // Open-loop calibration
#define VCO_MODE_CLOSE_LOOP_CALIBRATION 1 // Close-loop calibration
#define VCO_MODE_AMPLITUDE_CALIBRATION 2 // Amplitude calibration
#define VCO_MODE_OPEN_LOOP_CLOSE_LOOP_AMPLITUDE_CALIBRATION 3 // Open-loop calibration, close-loop calibration, and then amplitude calibration
#define VCO_MODE_CLOSE_LOOP_AMPLITUDE_CALIBRATION 4 // Close-loop calibration and then amplitude calibration

//
// Support HW management queue
//
#define SUPPORT_HW_MGMT_QUEUE(_pAd) (0)

//
// BB/RF PA mode configurations
//
#define BB_PA_MODE_CFG0 0x1214
#define BB_PA_MODE_CFG1 0x1218
#define RF_PA_MODE_CFG0 0x121C
#define RF_PA_MODE_CFG1 0x1220

//
// RF PA mode (RF_PA_MODE_XXX)
//
#define RF_PA_MODE_OFDM_EVM_LIMITED (0)
#define RF_PA_MODE_OFDM_MASK_LIMITED (1)
#define RF_PA_MODE_CCK_EVM_LIMITED (2)
#define RF_PA_MODE_CCK_MASK_LIMITED (3)

//
// Tx0 RF gain correction
//
#define TX0_RF_GAIN_CORRECT 0x13A0
#define TX1_RF_GAIN_CORRECT 0x13A4
#define TX0_RF_GAIN_ATTEN 0x13A8
#define TX1_RF_GAIN_ATTEN 0x13AC
#define TX_ALC_CFG_0 0x13B0
#define TX_ALC_CFG_1 0x13B4
#define TX_ALC_DBG_1 0x13B8
#define TX0_BB_GAIN_ATTEN 0x13C0
#define TX1_BB_GAIN_ATTEN 0x13C4
#define TX_ALC_VGA3 0x13C8

//
// Aux power configuration
//
#define AUX_CLK_CFG 0x120C

//
// Per-channel Tx power
//
typedef struct _PER_CHANNEL_TX_POWER
{
    UCHAR Channel; // Channel number
    CHAR Tx0Power; // Tx0 power
    CHAR Tx1Power; // Tx1 power
    CHAR Tx2Power; // Tx2 power
} PER_CHANNEL_TX_POWER, *PPER_CHANNEL_TX_POWER;

//
// Antenna mode configurations in EEPROM
//
#define EEPROM_ANT_MODE_CFG (0x23)

//
// The 5G channel index (LB/MB and MB/HB) for the initial VGA and RSSI calculation
//
#define EEPROM_5G_CHANNEL_INDEX_LB_MB (0xDC)
#define EEPROM_5G_CHANNEL_INDEX_MB_HB (0xDD)

//
// Antenna mode (ANT_MODE_XXX)
//
#define ANT_MODE_DUAL (0x01) // Chip is in dual antenna mode
#define ANT_MODE_SINGLE (0x02) // Chip is in single antenna mode

//
// COEXCFG3
//
#define COEXCFG3 (0x004C)

//
// The auto fall back table for VHT and legacy data rates
//
#define VHT_LG_FBK_CFG1 (0x1360)

#ifdef BIG_ENDIAN
typedef union _VHT_LG_FBK_CFG1_STRUC
{
    struct
    {
        ULONG Reserved:8; // Reserved
        ULONG VHT_1SS_MCS9_FBK:4; // Auto fall back MCS as VHT 1SS MCS = 9
        ULONG VHT_1SS_MCS8_FBK:4; // Auto fall back MCS as VHT 1SS MCS = 8
        ULONG CCK3_FBK:4; //Auto fall back MCS as previous TX rate is CCK 11Mbps
        ULONG CCK2_FBK:4; //Auto fall back MCS as previous TX rate is CCK 5.5Mbps
        ULONG CCK1_FBK:4; //Auto fall back MCS as previous TX rate is CCK 2Mbps
        ULONG CCK0_FBK:4; //Auto fall back MCS as previous TX rate is CCK 1Mbps
    } field;

    ULONG word;
} VHT_LG_FBK_CFG1_STRUC, *PVHT_LG_FBK_CFG1_STRUC;
#else
typedef union _VHT_LG_FBK_CFG1_STRUC
{
    struct
    {
        ULONG CCK0_FBK:4; //Auto fall back MCS as previous TX rate is CCK 1Mbps
        ULONG CCK1_FBK:4; //Auto fall back MCS as previous TX rate is CCK 2Mbps
        ULONG CCK2_FBK:4; //Auto fall back MCS as previous TX rate is CCK 5.5Mbps
        ULONG CCK3_FBK:4; //Auto fall back MCS as previous TX rate is CCK 11Mbps
        ULONG VHT_1SS_MCS8_FBK:4; // Auto fall back MCS as VHT 1SS MCS = 8
        ULONG VHT_1SS_MCS9_FBK:4; // Auto fall back MCS as VHT 1SS MCS = 9
        ULONG Reserved:8; // Reserved
    } field;
    
    ULONG word;
} VHT_LG_FBK_CFG1_STRUC, *PVHT_LG_FBK_CFG1_STRUC;
#endif

//
// Table of MCS power and PA mode
//
typedef struct _MCSPOWER_PAMODE_TABLE
{
    CHAR MCS_Power; // Per-MCS Tx power
    UCHAR RF_PA_Mode; // RF PA mode (for internal PA only)
} MCSPOWER_PAMODE_TABLE, *PMCSPOWER_PAMODE_TABLE;

//
// Period of requesting TSSI/temperature report and adjusting/updating TSSI/temperature
//
#define MLME_REQUEST_TSSI_REPORT(_pAd) (((_pAd->Mlme.PeriodicRound - (1 * MLME_TASK_EXEC_MULTIPLE)) % (2 * MLME_TASK_EXEC_MULTIPLE)) == 0) // 2 seconds
#define MLME_ADJUST_TSSI(_pAd) (((_pAd->Mlme.PeriodicRound - (2 * MLME_TASK_EXEC_MULTIPLE)) % (2 * MLME_TASK_EXEC_MULTIPLE)) == 0) // 3 seconds
#define MLME_REQUEST_TEMPERATURE_REPORT(_pAd) (((_pAd->Mlme.PeriodicRound - (3 * MLME_TASK_EXEC_MULTIPLE)) % (5 * MLME_TASK_EXEC_MULTIPLE)) == 0) // 4 seconds
#define MLME_UPDATE_TEMPERATURE(_pAd) (((_pAd->Mlme.PeriodicRound - (4 * MLME_TASK_EXEC_MULTIPLE)) % (5 * MLME_TASK_EXEC_MULTIPLE)) == 0) // 5 seconds

//
// Get the Tx type of the TSSI Info #1
//
#define GET_TSSI_INFO1_TX_TYPE(_TssiInfo1) (_TssiInfo1 & 0x07)

//
// Get the Tx0 channel initial transmission gain
//
#define GET_TX0_CH_INITIAL_TX_GAIN(_MacValue) (_MacValue & 0x0000003F)

//
// Get the current channel
//
#define GET_CURRENT_CHANNEL(_pAd) (_pPort->Channel)

//
// Get the current central channel
//
#define GET_CURRENT_CENTRAL_CHANNEL(_pAd) (_pPort->CentralChannel)

//
// Get the current bandwidth (BW_XXX)
//
#define GET_CURRENT_BW(_pAd) (_pPort->BBPCurrentBW)

//
// TSSI information is not ready.
//
#define TSSI_INFO_IS_NOT_READY(_BB_CORE_R34_VALUE) ((_BB_CORE_R34_VALUE & 0x00000010) == 0x00000010)

//
// Japan Tx filter coefficients enabled
//
#define JAPAN_TX_FILTER_COEFFICIENTS_ENABLED(_BbCoreR1) (_BbCoreR1 & 0x00000020)

//
// Get the static Tx power control
//
#define GET_STATIC_TX_POWER_CTRL(_BbTxbeR4) (_BbTxbeR4 & 0x00000003)

#define DEFAULT_BO (4)
#define LIN2DB_ERROR_CODE (-10000)

//
// Static Tx power control (STATIC_TX_POWER_CTRL_XXX)
//
#define STATIC_TX_POWER_CTRL_NOMINAL_TX_POWER (0)
#define STATIC_TX_POWER_CTRL_DROP_TX_POWER_BY_6DB (1)
#define STATIC_TX_POWER_CTRL_DROP_TX_POWER_BY_12DB (2)
#define STATIC_TX_POWER_CTRL_ADD_TX_POWER_BY_6DB (3)

//
// The the current channel band idex
//
// 2G: 0
// 5G: 1
//
#define GET_CURRENT_CH_BAND_IDX(_pAd) ((_pPort->Channel <= 14) ? (0) : (1))

//
// 2G/5G (CH_BANK_XXX)
//
#define CH_BANK_2G (0)
#define CH_BANK_5G (1)

//
// Check the Tx power upperbound
//
#define CHECK_TX_POWER_UPPERBOUND(_TxPower, _Upperbound) (_TxPower >= _Upperbound) ? \
            (_TxPower = _Upperbound) : \
            (_TxPower = _TxPower)

//
// Unlimited Tx power upperbound
//
#define TX_POWER_UPPERBOUND_UNLIMITED (50)

//
// Tx power upperbound - 2G/BW20
//
#define TX_POWER_UPPERBOUND_2G_BW20_CH1_CCK_DEFAULT (16)
#define TX_POWER_UPPERBOUND_2G_BW20_CH1_OFDM_DEFAULT (14)
#define TX_POWER_UPPERBOUND_2G_BW20_CH1_HT_DEFAULT (13)
#define TX_POWER_UPPERBOUND_2G_BW20_CH11_CCK_DEFAULT (16)
#define TX_POWER_UPPERBOUND_2G_BW20_CH11_OFDM_DEFAULT (14)
#define TX_POWER_UPPERBOUND_2G_BW20_CH11_HT_DEFAULT (13)

//
// Tx power upperbound - 2G/BW40
//
#define TX_POWER_UPPERBOUND_2G_BW40_CH3_CCK_DEFAULT (16)
#define TX_POWER_UPPERBOUND_2G_BW40_CH3_OFDM_DEFAULT (14)
#define TX_POWER_UPPERBOUND_2G_BW40_CH3_HT_DEFAULT (12)
#define TX_POWER_UPPERBOUND_2G_BW40_CH9_CCK_DEFAULT (TX_POWER_UPPERBOUND_UNLIMITED)
#define TX_POWER_UPPERBOUND_2G_BW40_CH9_OFDM_DEFAULT (TX_POWER_UPPERBOUND_UNLIMITED)
#define TX_POWER_UPPERBOUND_2G_BW40_CH9_HT_DEFAULT (12)
#define TX_POWER_UPPERBOUND_2G_BW40_CH11_CCK_DEFAULT (16)
#define TX_POWER_UPPERBOUND_2G_BW40_CH11_OFDM_DEFAULT (14)
#define TX_POWER_UPPERBOUND_2G_BW40_CH11_HT_DEFAULT (13)

//
// Tx power upperbound - 5G/BW20
//
#define TX_POWER_UPPERBOUND_5G_BW20_CH36_OFDM_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW20_CH36_HTVHT_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW20_CH64_OFDM_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW20_CH64_HTVHT_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW20_CH100_OFDM_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW20_CH100_HTVHT_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW20_CH140_OFDM_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW20_CH140_HTVHT_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW40_CH38_OFDM_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW40_CH38_HTVHT_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW40_CH62_OFDM_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW40_CH62_HTVHT_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW40_CH102_OFDM_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW40_CH102_HTVHT_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW40_CH134_OFDM_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW40_CH134_HTVHT_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW80_CH42_OFDM_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW80_CH42_HTVHT_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW80_CH58_OFDM_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW80_CH58_HTVHT_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW80_CH106_OFDM_DEFAULT (13)
#define TX_POWER_UPPERBOUND_5G_BW80_CH106_HTVHT_DEFAULT (13)

//
// 802.11b CCK TSSI information
//
typedef union _CCK_TSSI_INFO_EXT
{
    struct
    {
        UCHAR TxType:3; // 0 (tx_type = cck)
        UCHAR TxBwSel:2; // tx_bw_sel
        UCHAR Rate:2; // 11b rate
        UCHAR ShortPreamble:1; // short preamble
    } field;

    UCHAR   value;
} CCK_TSSI_INFO_EXT, *PCCK_TSSI_INFO_EXT;

//
// 802.11a/g OFDM TSSI information
//
typedef union _OFDM_TSSI_INFO_EXT
{
    struct
    {
        UCHAR TxType:3; // 1 (tx_type = legacy OFDM)
        UCHAR Reserved:1; // 0
        UCHAR Rate:4; // OFDM rate
    } field;

    UCHAR   value;
} OFDM_TSSI_INFO_EXT, *POFDM_TSSI_INFO_EXT;

//
// 802.11n HT TSSI information
//
typedef union _HT_TSSI_INFO_EXT
{
    union
    {
        struct
        {
            UCHAR TxType:3; // tx_type (2 = MM, 3 = GF)
            UCHAR TxBwSel:2; // tx_bw_sel
            UCHAR Reserved:1; // 0
            UCHAR STBC:2; // STBC
        } field;

        UCHAR value;
    } PartA;

    union
    {
        struct
        {
            UCHAR MCS:7; // MCS
            UCHAR BW:1; // BW
        } field;

        UCHAR value;
    } PartB;
} HT_TSSI_INFO_EXT, *PHT_TSSI_INFO_EXT;

//
// 802.11ac VHT TSSI information
//
typedef union _VHT_TSSI_INFO_EXT
{
    union
    {
        struct
        {
            UCHAR TxType:3; // tx_type (4 = VHT)
            UCHAR TxBwSel:2; // tx_bw_sel
            UCHAR Nsts:3; // Nsts
        } field;

        UCHAR value;
    } PartA;

    union
    {
        struct
        {
            UCHAR MCS:4; // MCS
            UCHAR STBC:1; // STBC
            UCHAR BW:2; // BW
            UCHAR Reserved:1; // 0
        } field;

        UCHAR value;
    } PartB;
} VHT_TSSI_INFO_EXT, *PVHT_TSSI_INFO_EXT;

//
// TSSI compensation table
//
typedef struct _TSSI_COMPENSATION_TABLE
{
    MCSPOWER_PAMODE_TABLE MCS32[2]; // [2G/5G] {MCS 32}
    MCSPOWER_PAMODE_TABLE CCK[4]; // {2G CCK}
    MCSPOWER_PAMODE_TABLE OFDM[2][8]; // [2G/5G] {OFDM}
    MCSPOWER_PAMODE_TABLE HT[2][8]; // [2G/5G] {HT}
    MCSPOWER_PAMODE_TABLE VHT[2][10]; // [2G/5G] {VHT}
} TSSI_COMPENSATION_TABLE, *PTSSI_COMPENSATION_TABLE;

//
// The EEPROM of Tx MCS7 target power on 2G/5G
//
#define EEPROM_TX_MCS7_TARGET_POWER_ON_2G (0xD0) // Tx MCS7 target power on 2G
#define EEPROM_TX_MCS7_TARGET_POWER_ON_5G (0xD2) // Tx MCS7 target power on 5G

//
// OFDM 54Mbps per-rate Tx power on 2G/5G
//
#define EEPROM_PER_RATE_TX_POWER_OFDM_54M_ON_2G (0xE3)
#define EEPROM_PER_RATE_TX_POWER_OFDM_54M_ON_5G (0x123)

//
// BW20/40/80 Tx power delta
//
#define EEPROM_2040BW_TX_POWER_DELTA_ON_2G (0x50) // BW20/40 Tx power delta on 2G
#define EEPROM_2040BW_TX_POWER_DELTA_ON_5G (0x51) // BW20/40 Tx power delta on 5G
#define EEPROM_2080BW_TX_POWER_DELTA_ON_5G (0xD3) // BW20/80 Tx power delta on 5G

//
// TSSI slope and offset
//
#define EEPROM_TSSI_SLOPE_ON_2G (0x6E) // TSSI slope on 2G
#define EEPROM_TSSI_OFFSET_ON_2G (0x6F) // TSSI offset on 2G
#define EEPROM_TSSI_SLOPE_ON_5G_GROUP1 (0xF0) // TSSI slope on 5G (group 1)
#define EEPROM_TSSI_OFFSET_ON_5G_GROUP1 (0xF1) // TSSI offset on 5G (group 1)
#define EEPROM_TSSI_SLOPE_ON_5G_GROUP2 (0xF2) // TSSI slope on 5G (group 2)
#define EEPROM_TSSI_OFFSET_ON_5G_GROUP2 (0xF3) // TSSI offset on 5G (group 2)
#define EEPROM_TSSI_SLOPE_ON_5G_GROUP3 (0xF4) // TSSI slope on 5G (group 3)
#define EEPROM_TSSI_OFFSET_ON_5G_GROUP3 (0xF5) // TSSI offset on 5G (group 3)
#define EEPROM_TSSI_SLOPE_ON_5G_GROUP4 (0xF6) // TSSI slope on 5G (group 4)
#define EEPROM_TSSI_OFFSET_ON_5G_GROUP4 (0xF7) // TSSI offset on 5G (group 4)
#define EEPROM_TSSI_SLOPE_ON_5G_GROUP5 (0xF8) // TSSI slope on 5G (group 5)
#define EEPROM_TSSI_OFFSET_ON_5G_GROUP5 (0xF9) // TSSI offset on 5G (group 5)
#define EEPROM_TSSI_SLOPE_ON_5G_GROUP6 (0xFA) // TSSI slope on 5G (group 6)
#define EEPROM_TSSI_OFFSET_ON_5G_GROUP6 (0xFB) // TSSI offset on 5G (group 6)
#define EEPROM_TSSI_SLOPE_ON_5G_GROUP7 (0xFC) // TSSI slope on 5G (group 7)
#define EEPROM_TSSI_OFFSET_ON_5G_GROUP7 (0xFD) // TSSI offset on 5G (group 7)
#define EEPROM_TSSI_SLOPE_ON_5G_GROUP8 (0xFE) // TSSI slope on 5G (group 8)
#define EEPROM_TSSI_OFFSET_ON_5G_GROUP8 (0xFF) // TSSI offset on 5G (group 8)
#define EEPROM_TSSI_SLOPE_ON_G_11J_BAND (0xDA) // TSSI slope on 11J band
#define EEPROM_TSSI_OFFSET_ON_G_11J_BAND (0xDB) // TSSI offset on 11J band

//
// TSSI 5G group boundary
//
#define EEPROM_TSSI_5G_GROUP1_BOUNDARY (0xD4)
#define EEPROM_TSSI_5G_GROUP2_BOUNDARY (0xD5)
#define EEPROM_TSSI_5G_GROUP3_BOUNDARY (0xD6)
#define EEPROM_TSSI_5G_GROUP4_BOUNDARY (0xD7)
#define EEPROM_TSSI_5G_GROUP5_BOUNDARY (0xD8)
#define EEPROM_TSSI_5G_GROUP6_BOUNDARY (0xD9)
#define EEPROM_TSSI_5G_GROUP7_BOUNDARY (0xDA)

//
// RF bypass #0
//
#define RF_BYPASS_0 (0x0504)

//
// RF setting #0
//
#define RF_SETTING_0 (0x050C)

//
// The entry of transmit power control over MAC
//
typedef struct _TX_POWER_CONTROL_OVER_MAC_ENTRY
{
    USHORT  MACRegisterOffset; // MAC register offset
    ULONG   RegisterValue; // Register value
} TX_POWER_CONTROL_OVER_MAC_ENTRY, *PTX_POWER_CONTROL_OVER_MAC_ENTRY;

//
// The maximum registers of transmit power control
//
#define MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS   15
#define MAX_TX_PWR_CONTROL_OVER_VHT_MAC_REGISTERS   8

//
// The configuration of the transmit power control over MAC
//
typedef struct _CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC
{
    UCHAR                               NumOfEntries; // Number of entries
    TX_POWER_CONTROL_OVER_MAC_ENTRY TxPwrCtrlOverMAC[MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS];
} CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC, *PCONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC;

//
// The per-rate Tx power table entry
//
typedef struct _PER_RATE_TX_POWER_TABLE_ENTRY
{
    UCHAR CentralChannel; // Central channel
    UCHAR BW; // Bandwidth (BW_XX)
    CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC PerRatePowerRegSettings; // Per-rate Tx power register settings
} PER_RATE_TX_POWER_TABLE_ENTRY, *PPER_RATE_TX_POWER_TABLE_ENTRY;

//
// Number of entries in 2G/5G per-rate Tx power table
//
#define NUM_OF_ENTRIES_2G_PER_RATE_TX_POWER_TABLE (5)
#define NUM_OF_ENTRIES_5G_PER_RATE_TX_POWER_TABLE (11)

//
// The per-rate Tx power table
//
typedef struct _PER_RATE_TX_POWER_TABLE
{
    PER_RATE_TX_POWER_TABLE_ENTRY PowerTable2G[NUM_OF_ENTRIES_2G_PER_RATE_TX_POWER_TABLE];
    PER_RATE_TX_POWER_TABLE_ENTRY PowerTable5G[NUM_OF_ENTRIES_5G_PER_RATE_TX_POWER_TABLE];

    CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC PowerTable2GDefault;
    CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC PowerTable5GGroup1Default;
    CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC PowerTable5GGroup2Default;
    CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC PowerTable5GGroup3Default;
    CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC PowerTable5GGroup4Default;
} PER_RATE_TX_POWER_TABLE, *PPER_RATE_TX_POWER_TABLE;

//
// The Tx power control using the internal ALC
//
typedef struct _TX_POWER_CONTROL_EXT
{
    TSSI_COMPENSATION_TABLE TssiTable; // TSSI compensation table

    UCHAR TxMcs7TargetPowerOn2G; // 2G Tx target power
    UCHAR TxMcs7TargetPowerOn5G; // 5G Tx target power

    CHAR TxPowerDelta2040BWOn2G; // BW20/40 Tx power delta on 2G
    CHAR TxPowerDelta2040BWOn5G; // BW20/40 Tx power delta on 5G
    CHAR TxPowerDelta2080BWOn5G; // BW20/80 Tx power delta on 5G

    UCHAR TssiSlopeOn2G; // TSSI slope on 2G
    CHAR TssiOffsetOn2G; // TSSI offset on 2G
    UCHAR TssiSlopeOn5GGroup1; // TSSI slope on 5G - group 1
    CHAR TssiOffsetOn5GGroup1; // TSSI offset on 5G - group 1
    UCHAR TssiSlopeOn5GGroup2; // TSSI slope on 5G - group 2
    CHAR TssiOffsetOn5GGroup2; // TSSI offset on 5G - group 2
    UCHAR TssiSlopeOn5GGroup3; // TSSI slope on 5G - group 3
    CHAR TssiOffsetOn5GGroup3; // TSSI offset on 5G - group 3
    UCHAR TssiSlopeOn5GGroup4; // TSSI slope on 5G - group 4
    CHAR TssiOffsetOn5GGroup4; // TSSI offset on 5G - group 4
    UCHAR TssiSlopeOn5GGroup5; // TSSI slope on 5G - group 5
    CHAR TssiOffsetOn5GGroup5; // TSSI offset on 5G - group 5
    UCHAR TssiSlopeOn5GGroup6; // TSSI slope on 5G - group 6
    CHAR TssiOffsetOn5GGroup6; // TSSI offset on 5G - group 6
    UCHAR TssiSlopeOn5GGroup7; // TSSI slope on 5G - group 7
    CHAR TssiOffsetOn5GGroup7; // TSSI offset on 5G - group 7
    UCHAR TssiSlopeOn5GGroup8; // TSSI slope on 5G - group 8
    CHAR TssiOffsetOn5GGroup8; // TSSI offset on 5G - group 8

    UCHAR TssiSlopeOn11jBand; // TSSI slope on 11j band
    CHAR TssiOffsetOn11jBand; // TSSI offset on 11j band

    UCHAR Tssi5GGroup1Boundary; // TSSI 5G group 1 boundary
    UCHAR Tssi5GGroup2Boundary; // TSSI 5G group 2 boundary
    UCHAR Tssi5GGroup3Boundary; // TSSI 5G group 3 boundary
    UCHAR Tssi5GGroup4Boundary; // TSSI 5G group 4 boundary
    UCHAR Tssi5GGroup5Boundary; // TSSI 5G group 5 boundary
    UCHAR Tssi5GGroup6Boundary; // TSSI 5G group 6 boundary
    UCHAR Tssi5GGroup7Boundary; // TSSI 5G group 7 boundary

    CHAR PerRateTxPowerOfdm54MOn2G; // OFDM 54Mbps per-rate Tx power
    CHAR PerRateTxPowerOfdm54MOn5G; // OFDM 54Mbps per-rate Tx power

    CHAR TssiDcOn2G; // TSSI DC on 2G
    CHAR TssiDcOn5G; // TSSI DC on 5G

    LONG TssiDeltaPre; // Privous TSSI delta

    //
    // Backup register values for DC calibration
    //
    UCHAR RfB0R67; // RF bank 0, R67
    ULONG MacRfSetting0; // MAC RF_SETTING0
    ULONG BbCoreR4; // BB CORE, R4
    ULONG BbCoreR34; // BB CORE, R34
    ULONG MacRfBypass0; // MAC RF_BYPASS0
    ULONG BbTxbeR6; // BB TXBE R6   

    //
    // Per-rate Tx power table
    //
    PER_RATE_TX_POWER_TABLE PerRateTxPowerTable;

    //
    // Current per-rate Tx power control over MAC
    //
    PCONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC pCurrentPerRateTxPowerCtrlOverMac;
    
} TX_POWER_CONTROL_EXT, *PTX_POWER_CONTROL_EXT;


//
// MT76XX Temperature Sensor
//
#define EEPROM_TEMPOFFSET (0xD0)

//
// Tx/Rx IQ calibration configuration entry
//
typedef struct _TXRX_IQ_CAL_CFG_ENTRY
{
    UCHAR GainImb; // Amplitude I/Q adjustment
    UCHAR PhaseImb; // Phase adjustment
    UCHAR GroupDelay; // Group delay imbalance
} TXRX_IQ_CAL_CFG_ENTRY, *PTXRX_IQ_CAL_CFG_ENTRY;

//
// Tx/Rx IQ calibration configuration table
//
typedef struct _TXRX_IQ_CAL_CFG_TABLE
{
    TXRX_IQ_CAL_CFG_ENTRY TxIQCalCfgGBand; // Tx IQ calibration configuration for G band
    TXRX_IQ_CAL_CFG_ENTRY TxIQCalCfgABandLB; // Tx IQ calibration configuration for A band, channel lower (36~64)
    TXRX_IQ_CAL_CFG_ENTRY TxIQCalCfgABandMB; // Tx IQ calibration configuration for A band, channel middle (100~136)
    TXRX_IQ_CAL_CFG_ENTRY TxIQCalCfgABandHB; // Tx IQ calibration configuration for A band, channel upper (137~173)

    TXRX_IQ_CAL_CFG_ENTRY RxIQCalCfgGBand; // Rx IQ calibration configuration for G band
    TXRX_IQ_CAL_CFG_ENTRY RxIQCalCfgABandLB; // Rx IQ calibration configuration for A band, channel lower (36~64)
    TXRX_IQ_CAL_CFG_ENTRY RxIQCalCfgABandMB; // Rx IQ calibration configuration for A band, channel middle (100~136)
    TXRX_IQ_CAL_CFG_ENTRY RxIQCalCfgABandHB; // Rx IQ calibration configuration for A band, channel upper (137~173)
} TXRX_IQ_CAL_CFG_TABLE, *PTXRX_IQ_CAL_CFG_TABLE;

//
// The eFuse locations of the Tx/Rx IQ calibration configuratin table (TX_IQ_CAL_EFUSE_CFG_XXX and RX_IQ_CAL_EFUSE_CFG_XXX)
//
#define TX_IQ_CAL_EFUSE_CFG_GAIN_IMB_GBAND (0xFA) // Tx gain imb for G band
#define TX_IQ_CAL_EFUSE_CFG_GAIN_IMB_ABAND_LB (0xFB) // Tx gain imb for A band, channel lower (36~64)
#define TX_IQ_CAL_EFUSE_CFG_GAIN_IMB_ABAND_MB (0xFC) // Tx gain imb for A band, channel middle (100~136)
#define TX_IQ_CAL_EFUSE_CFG_GAIN_IMB_ABAND_HB (0xFD) // Tx gain imb for A band, channel upper (137~173)
#define TX_IQ_CAL_EFUSE_CFG_PHASE_IMB_GBAND (0xFF) // Tx phase imb for G band
#define TX_IQ_CAL_EFUSE_CFG_PHASE_IMB_ABAND_LB (0x100) // Tx phase imb for A band, channel lower (36~64)
#define TX_IQ_CAL_EFUSE_CFG_PHASE_IMB_ABAND_MB (0x101) // Tx phase imb for A band, channel middle (100~136)
#define TX_IQ_CAL_EFUSE_CFG_PHASE_IMB_ABAND_HB (0x102) // Tx phase imb for A band, channel upper (137~173)
#define TX_IQ_CAL_EFUSE_CFG_GROUP_DELAY_GBAND (0x104) // Tx group delay for G band
#define TX_IQ_CAL_EFUSE_CFG_GROUP_DELAY_ABAND_LB (0x105) // Tx group delay for A band, channel lower (36~64)
#define TX_IQ_CAL_EFUSE_CFG_GROUP_DELAY_ABAND_MB (0x106) // Tx group delay for A band, channel middle (100~136)
#define TX_IQ_CAL_EFUSE_CFG_GROUP_DELAY_ABAND_HB (0x107) // Tx group delay for A band, channel upper (137~173)
#define RX_IQ_CAL_EFUSE_CFG_GAIN_IMB_GBAND (0x109) // Rx gain imb for G band
#define RX_IQ_CAL_EFUSE_CFG_GAIN_IMB_ABAND_LB (0x10A) // Rx gain imb for A band, channel lower (36~64)
#define RX_IQ_CAL_EFUSE_CFG_GAIN_IMB_ABAND_MB (0x10B) // Rx gain imb for A band, channel middle (100~136)
#define RX_IQ_CAL_EFUSE_CFG_GAIN_IMB_ABAND_HB (0x10C) // Rx gain imb for A band, channel upper (137~173)
#define RX_IQ_CAL_EFUSE_CFG_PHASE_IMB_GBAND (0x10E) // Rx phase imb for G band
#define RX_IQ_CAL_EFUSE_CFG_PHASE_IMB_ABAND_LB (0x10F) // Rx phase imb for A band, channel lower (36~64)
#define RX_IQ_CAL_EFUSE_CFG_PHASE_IMB_ABAND_MB (0x110) // Rx phase imb for A band, channel middle (100~136)
#define RX_IQ_CAL_EFUSE_CFG_PHASE_IMB_ABAND_HB (0x111) // Rx phase imb for A band, channel upper (137~173)
#define RX_IQ_CAL_EFUSE_CFG_GROUP_DELAY_GBAND (0x113) // Rx group delay for G band
#define RX_IQ_CAL_EFUSE_CFG_GROUP_DELAY_ABAND_LB (0x114) // Rx group delay for A band, channel lower (36~64)
#define RX_IQ_CAL_EFUSE_CFG_GROUP_DELAY_ABAND_MB (0x115) // Rx group delay for A band, channel middle (100~136)
#define RX_IQ_CAL_EFUSE_CFG_GROUP_DELAY_ABAND_HB (0x116) // Rx group delay for A band, channel upper (137~173)

#pragma pack(pop, struct_pack1)

//
// Read the DoubleWord of the PHY_CFG
//
ULONG FORCEINLINE READ_PHY_CFG_DOUBLE_WORD(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg);

//
// Read the MCS of the PHY_CFG
//
ULONG FORCEINLINE READ_PHY_CFG_MCS(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg);

//
// Read the BW of the PHY_CFG
//
ULONG FORCEINLINE READ_PHY_CFG_BW(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg);

//
// Read the ShortGI of the PHY_CFG
//
ULONG FORCEINLINE READ_PHY_CFG_SHORT_GI(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg);

//
// Read the STBC of the PHY_CFG
//
ULONG FORCEINLINE READ_PHY_CFG_STBC(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg);

//
// Read the bITxBfPacket of the PHY_CFG
//
ULONG FORCEINLINE READ_PHY_CFG_ITX_BF_PACKET(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg);

//
// Read the bETxBfPacket of the PHY_CFG
//
ULONG FORCEINLINE READ_PHY_CFG_ETX_BF_PACKET(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg);

//
// Read the Mode of the PHY_CFG
//
ULONG FORCEINLINE READ_PHY_CFG_MODE(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg);

//
// Read the Nss of the PHY_CFG
//
ULONG FORCEINLINE READ_PHY_CFG_NSS(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg);

//
// Write the DoubleWord of PHY_CFG
//
VOID FORCEINLINE WRITE_PHY_CFG_DOUBLE_WORD(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG DoubleWord);

//
// Write the MCS of the PHY_CFG
//
VOID FORCEINLINE WRITE_PHY_CFG_MCS(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG MCS); // MCS_XXX

//
// Write the BW of the PHY_CFG
//
VOID FORCEINLINE WRITE_PHY_CFG_BW(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG BW); // BW_XXX

//
// Write the ShortGI of the PHY_CFG
//
VOID FORCEINLINE WRITE_PHY_CFG_SHORT_GI(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG ShortGI); // GI_XXX

//
// Write the STBC of the PHY_CFG
//
VOID FORCEINLINE WRITE_PHY_CFG_STBC(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG STBC); // STBC_XXX

//
// Write the bITxBfPacket of the PHY_CFG
//
VOID FORCEINLINE WRITE_PHY_CFG_ITX_BF_PACKET(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG bITxBfPacket); // 1/0

//
// Write the bETxBfPacket of the PHY_CFG
//
VOID FORCEINLINE WRITE_PHY_CFG_ETX_BF_PACKET(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG bETxBfPacket); // 1/0

//
// Write the Mode of the PHY_CFG
//
VOID FORCEINLINE WRITE_PHY_CFG_MODE(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG Mode); // MODE_XXX

//
// Write the Nss of the PHY_CFG
//
VOID FORCEINLINE WRITE_PHY_CFG_NSS(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pPhyCfg, 
    IN ULONG Nss); // NSS_XXX

// Print the member of the PHY_CFG
//
extern VOID FORCEINLINE PRINT_PHY_CFG(
    IN PMP_ADAPTER    pAd, 
    IN PPHY_CFG         pPhyCfg,
    IN ULONG            RTDebugLevel);

//
// Indicate rate for UI
//
VOID IndicateRateForUi(
    IN PMP_ADAPTER pAd, 
    IN PPHY_CFG pRatePhyCfg, 
    IN OUT PVOID pRateIndication, 
    IN OUT PULONG pRateSize);

//
// Read the IQ compensation configuration from eFuse
//
VOID ReadIQCompensationConfiguraiton(
    IN PMP_ADAPTER pAd);

//
// Set the Tx/Rx bandwidth control
//
VOID BbSetTxRxBwCtrl(
    IN PMP_ADAPTER pAd, 
    IN UCHAR TxRxBwCtrl); // BW_XXX


//
// Switch channel bandwidth
//
VOID SwitchBandwidth(
    IN PMP_ADAPTER pAd, 
    IN BOOLEAN bSwitchChannel, 
    IN UCHAR Channel, 
    IN UCHAR Bw, // BW_XXX
    IN UCHAR ExtensionChannelOffset); // EXTCHA_XXX

//
// Set the Tx DAC control based on the number of Tx paths
//
VOID BbSetTxDacCtrlByTxPath(
    IN PMP_ADAPTER pAd);

//
// DAC control
//
#define DAC_0                   (0)
#define DAC_1                   (1)
#define DAC_0_AND_DAC_1     (2)

//
// Set the Tx DAC control
//
VOID BbSetTxDacCtrl(
    IN PMP_ADAPTER pAd, 
    IN UCHAR DacCtrl); // DAC_XXX

//
// Number of Rx antenna
//
#define RX_ANT_1R   (0)
#define RX_ANT_2R   (1)
#define RX_ANT_3R   (2)

//
// Set Rx antenna
//
VOID BbSetRxAnt(
    IN PMP_ADAPTER pAd, 
    IN UCHAR RxAntNumber); // RX_ANT_XXX

//
// ADC control
//
#define ADC_0                   (0)
#define ADC_1                   (1)
#define ADC_0_AND_ADC_1     (2)

//
// Set Rx ADC
//
VOID BbSetRxAdc(
    IN PMP_ADAPTER pAd, 
    IN UCHAR AdcCtrl); // ADC_XXX

//
// Adjust the initial Rx AGC VGA value
//
VOID BbAdjustRxAgcVga(
    IN PMP_ADAPTER pAd, 
    IN BOOLEAN bApMode);

//
// Update Rx AGC VGA value for long distance
//
VOID BbUpdateRxAgcVgaForLongDistance(
    IN PMP_ADAPTER pAd); // Update for long distance

//
// Turn on MLD for 2x2 NIC
//
VOID BbSetMldFor2Stream(
    IN PMP_ADAPTER pAd, 
    IN BOOLEAN bEnable);

//
// In VHT, use primary ch, BW, VhtCentralChannel to calculate Primary channel location
//
VOID GetPrimaryChannelLocation(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           PrimaryChannel,     // Primary Channel Index
    IN  UCHAR           BW,                 // Bandwidth
    IN  UCHAR           CentralChannel,     // Central Channel Index
    OUT PUCHAR          pPrimaryChLocation);

//
// Temperature state (TEMPERATURE_STATE_XXX)
//
#define TEMPERATURE_STATE_NORMAL (0)
#define TEMPERATURE_STATE_HIGH (1)
#define TEMPERATURE_STATE_LOW (2)

//
// The period, in seconds, of updating BB registers based on temperature
//
#define PERIOD_OF_UPDATING_BB_REGISTERS_BASED_ON_TEMPERATURE (5)

//
// The Tx/Rx mode of the IQ calibration (IQ_CAL_MODE_XXX)
//
#define IQ_CAL_MODE_TX (0)
#define IQ_CAL_MODE_RX (1)

//
// Save the information of the current bandwidth and primary 20 location
//
VOID SaveCurrentBwPrimary20LocationInfo(
    IN PMP_ADAPTER pAd);

//
// Disable BW Signaling
//
VOID DisableBwSignaling(
    IN PMP_ADAPTER pAd);

//
// BW Signaling method (BW_SIGNALING_METHOD_XXX)
//
#define BW_SIGNALING_METHOD_STATIC (0)
#define BW_SIGNALING_METHOD_DYNAMIC (1)

//
// Enable BW Signaling
//
VOID EnableBwSignaling(
    IN PMP_ADAPTER pAd, 
    IN ULONG BwSignalingMethod); // BW_SIGNALING_METHOD_XXX

//
// Disallow TKIP with VHT rates
//
VOID DisallowTkipWithVhtRates(
    IN PMP_ADAPTER pAd);

//
// Allow TKIP with VHT rates
//
VOID AllowTkipWithVhtRates(
    IN PMP_ADAPTER pAd);

//
// Send the Operating Mode Notificaiton Action
//
VOID SendOperatingModeNotificaitonAction(
    IN PMP_ADAPTER pAd, 
    PUCHAR pDA, // Destination address, 
    UCHAR ChannelWidth, // Channel width (CH_WIDTH_BWXXX)
    UCHAR RxNss, // Rx Nss (RX_NSS_XXX)
    UCHAR RxNssType); // Rx Nss Type (RX_NSS_TYPE_MAX_RX_NSS_XXX)

//
// Add the Operating Mode Notification in the Association Request
//
VOID AddOperatingModeNotificationInAssociationRequest(
    IN PMP_ADAPTER pAd);

//
// Remove the Operating Mode Notification from the Association Request
//
VOID RemoveOperatingModeNotificationFromAssociationRequest(
    IN PMP_ADAPTER pAd);

#endif // __RT6X9X_H__
//////////////////////////////////// End of File ////////////////////////////////////

