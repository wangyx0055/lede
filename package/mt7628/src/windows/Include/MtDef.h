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
    rtmp_def.h

    Abstract:
    Miniport related definition header

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    08-01-2002    created
    John Chang  08-05-2003    add definition for 11g & other drafts
*/
#ifndef __MTDEF_H__
#define __MTDEF_H__

//#ifdef NDIS620_MINIPORT
// Maximum number of ports that can be created on this adapter (excludes the helper port)
//#define RTMP_MAX_NUMBER_OF_PORT   3
//#endif

//#ifdef NDIS630_MINIPORT
#define RTMP_MAX_NUMBER_OF_PORT             5   // STA, AP, WFD_Device, WFD_Role
//#endif

// Support Ralink WFD on Win8
//#define WIN8_RALINK_WFD_SUPPORT

// Assign Port's number
#define PORT_0          0   // Port0: STA, SoftAP, P2P
#define PORT_1          1   // Port1: ViwifiAP, WFD Device
#define PORT_2          2   // Port2:  VirtualSTA for P2P, ViwifiAP, WFD Device

#define PORT_3          3   // Port3: WFD GO/Client Role Port
#define PORT_4          4   // Port4: WFD GO/Client Role Port

#define FXXK_PORT_0    0   // Need remove
#define FXXK_PORT_1    1   // Need remove

#define MAC_ADDR_INDEX_0 0
#define MAC_ADDR_INDEX_1 1
#define MAC_ADDR_INDEX_2 2
#define MAC_ADDR_INDEX_3 3
#define MAC_ADDR_INDEX_4 4
#define MAC_ADDR_INDEX_5 5
#define MAC_ADDR_INDEX_6 6
#define MAC_ADDR_INDEX_7 7

//
//  Debug information verbosity: lower values indicate higher urgency
//
#define RT_DEBUG_OFF        1
#define RT_DEBUG_ERROR      2
#define RT_DEBUG_WARN       3
#define RT_DEBUG_TRACE      4
#define RT_DEBUG_INFO       5
#define RT_DEBUG_LOUD       6
#define RT_DEBUG_EMU      2

// TODO: need modify...
#define DEBUG_OFF          1
#define DEBUG_ERROR      2
#define DEBUG_WARN       3
#define DEBUG_TRACE      4
#define DEBUG_INFO        5
#define DEBUG_LOUD        6

#define DBG_LVL_OFF          1
#define DBG_LVL_ERROR      2
#define DBG_LVL_WARN       3
#define DBG_LVL_TRACE      4
#define DBG_LVL_INFO        5
#define DBG_LVL_LOUD        6

#if (!COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))
#define NDIS_DEFAULT_PORT_NUMBER 0
#endif

typedef enum{
	DBG_CAT_INIT 	= 0x00000001,	/* init/shutdown */
	DBG_CAT_HW		= 0x00000002,	/* MAC/BBP/RF/Chip */
	DBG_CAT_FW		= 0x00000004,
	DBG_CAT_INTF	= 0x00000008,	/* interface: usb/sdio/pcie/rbus */
	DBG_CAT_FPGA	= 0x00000010,
	DBG_CAT_ATE		= 0x00000020,
	DBG_CAT_RA		= 0x00000040,
	DBG_CAT_MODE	= 0x00000080,	/* MBSSID/apcli/mesh/p2p */
	DBG_CAT_AP		= 0x00000100,
	DBG_CAT_STA		= 0x00000200,
	DBG_CAT_TX		= 0x00000400,
	DBG_CAT_RX		= 0x00000800,
	DBG_CAT_CFG		= 0x00001000,	/* ioctl/profile/cfg80211 */
	DBG_CAT_MLME	= 0x00002000,
	DBG_CAT_PTCL	= 0x00004000,	/* protocol/function */
	DBG_CAT_MGMT	= 0x00008000,
	DBG_CAT_SEC		= 0x00010000,	/* security/key/wps */
	DBG_CAT_PS		= 0x00020000,	/* power saving/UAPSD */
	DBG_CAT_ALL		= 0xFFFFFFFF
}DEBUG_CATEGORY;

#define DBG_SUBCAT_ALL	0xFFFFFFFF


#define NIC_TAG             ((ULONG)'3067')
#define NIC_TAG_TX          ((ULONG)'T782')
#define NIC_TAG_RX          ((ULONG)'R782')
#define NIC_TAG_INFO    ((ULONG)'I782')
#define NIC_TAG_DBG     ((ULONG)'D782')

#define NIC_TAG1                ((ULONG)'1782')
#define NIC_TAG2                ((ULONG)'2782')
#define NIC_TAG3                ((ULONG)'3782')
#define NIC_TAG4                ((ULONG)'4782')
#define NIC_TAG5                ((ULONG)'5782')
#define NIC_TAG6                ((ULONG)'6782')
#define NIC_TAG7                ((ULONG)'7782')
#define NIC_TAG8                ((ULONG)'8782')
#define NIC_TAG9                ((ULONG)'9782')
#define NIC_TAG10           ((ULONG)'a782')
#define NIC_TAG11           ((ULONG)'b782')
#define NIC_TAG12           ((ULONG)'c782')
#define NIC_TAG13           ((ULONG)'e782')
#define NIC_TAG14           ((ULONG)'f782')
#define NIC_TAG15           ((ULONG)'g782')
#define NIC_TAG16           ((ULONG)'h782')
#define NIC_TAG17           ((ULONG)'i782')
#define NIC_TAG18           ((ULONG)'j782')
#define NIC_TAG19           ((ULONG)'k782')
#define NIC_TAG20           ((ULONG)'l782')
#define NIC_TAG21           ((ULONG)'m782')
#define NIC_TAG22           ((ULONG)'n782')
#define NIC_TAG23           ((ULONG)'o782')
#define NIC_TAG24           ((ULONG)'p782')
#define NIC_TAG25           ((ULONG)'q782')
#define NIC_TAG26           ((ULONG)'r782')
#define MT76X1_TAG           ((ULONG)'lx67') // Tx power control extension

// for CCX
#define NIC_TAGCCX             ((ULONG)'x782')

// for MLME Allocate Memory
#define MLME_MEMORY_TAG     0x0E8D0E8D

// For TDT
#define NIC_TAGTDT             ((ULONG)'y782')
#define NIC_DBG_STRING      ("**RT2870**")
#define MODEL_NUMBER        ("RT2870")

#define PACKED

//
// NDIS media type, current is ethernet, change if native wireless supported
//
#define NIC_MEDIA_TYPE          NdisMedium802_3
#define NIC_PCI_HDR_LENGTH      0xe2
#define NIC_MAX_PACKET_SIZE     2304
#define NIC_HEADER_SIZE         14

#define RETRY_LIMIT 10

//
// Entry number for each DMA descriptor ring
//
#define RXBULKAGGRE_SIZE        13
#define ROUGH_LEN       60  

// For Tx Buffer  
#define SEGMENT_TOTAL       15  // How many segments does the total tx buffer break into ?  Must be  represented by TxInfo->SwUseIdx.
#define ONE_SEGMENT     15  // Make sure that ONE_SEGMENT_UNIT > bulkout aggregation size + one wireless packet
#define ONE_SEGMENT_UNIT        (LOCAL_TXBUF_SIZE * ONE_SEGMENT) // How large is one segment ?
#define MAX_TXBULK_SIZE         (ONE_SEGMENT_UNIT * SEGMENT_TOTAL)  
#define MAX_TXBULK_LIMIT        (MAX_TXBULK_SIZE - LOCAL_TXBUF_SIZE)    
#define MAX_TXAMSDUBULK_LIMIT       (MAX_TXBULK_SIZE - (3*LOCAL_TXBUF_SIZE))    

#define MAX_RXBULK_SIZE         (LOCAL_TXBUF_SIZE*RXBULKAGGRE_SIZE) 
#define RX_BUFFER_NORMSIZE      4096
#define TX_RING_SIZE            1
#define PRIO_RING_SIZE          40

//Final Ring Index is for BulkInFail Data. So, if we want to increase Rx ring size, just modify RX_RING_SIZE
#define RX_RING_SIZE                        32//16
#define RX_RING_SIZE_END_INDEX       RX_RING_SIZE -1

// If bMultipleIRP is TRUE, we will use MULTIPLE_BULKIN_NUM
#define MULTIPLE_BULKIN_NUM     2

#define BEACON_RING_SIZE        2
#define MGMT_RING_SIZE          PRIO_RING_SIZE
#define PRIO_BUFFER_SIZE        1024  // 2048
#define BUFFER_SIZE             2400    //2048
#define MAX_FRAME_SIZE          2346                    // Maximum 802.11 frame size
#define MAX_ETHFRAME_SIZE           1500                    // Maximum 802.11 frame size
#define ALLOC_RX_PACKET_POOL    (RX_RING_SIZE)
#define ALLOC_RX_BUFFER_POOL    (ALLOC_RX_PACKET_POOL)
#define TX_RING                 0xa
#define ATIM_RING               0xb
#define PRIO_RING               0xc
#define RX_RING                 0xd
#define BEACON_RING             0xe
#define NULL_RING               0xf
#define MAX_TX_PROCESS          64 //32 //8
#define MAX_RX_PROCESS          4
#define MAX_CLIENT              4
#define MAX_SIZE_OF_MCAST_PSQ   (MAX_NUM_OF_FREE_NDIS_PACKET >> 2) // AP won't spend more than 1/4 of total buffers on M/BCAST PSQ
#define NUM_OF_LOCAL_RX_PKT_BUFFER  96
#define NDIS_IO_WAIT 200
#define RATXAGG_TH_EN           1000
#define MAX_AIRONET_FRAME_SIZE  2048
#define MAX_AIRONET_CLIENT_CMD_SIZE  256

// RxFilter
#define STANORMAL    0x17f97
#define APNORMAL     0x14f97
#define PSPXLINK     0x17f93
#define APCLIENTNORMAL   0x14f93

#define MAX_LEN_OF_MLME_BUFFER  2048
#define MAX_MLME_HANDLER_MEMORY           20    //each them cantains  MAX_LEN_OF_MLME_BUFFER size 

#define LOCAL_TXBUF_SIZE_4K 4096
#define LOCAL_TXBUF_SIZE        2048
#define TXD_SIZE                64
#define RXD_SIZE                64
#define TX_DMA_1ST_BUFFER_SIZE  64    // only the 1st physical buffer is pre-allocated
#define MGMT_DMA_BUFFER_SIZE    2048
#define RX_DMA_BUFFER_SIZE      4096
#define MAX_AGGREGATION_SIZE    8192
#define MAX_FW_SIZE     14356  // 14KB + sizeof(FIRMWARE_TXDSCR) + 4bytes padding for end aggreagtion
#define MAX_DMA_DONE_PROCESS    TX_RING_SIZE
#define MAX_TX_DONE_PROCESS     8
#define MAX_NUM_OF_TUPLE_CACHE  2
#define MAX_MCAST_LIST_SIZE     32
#define MAX_LEN_OF_VENDOR_DESC  64
#define NIC_RTMP_ADAPTER_NAME_SIZE          128
 
#define NUM_OF_MP_XMIT_CTRL_UNIT_BUF  128
#define NUM_OF_MP_XMIT_CTRL_UNIT_NUMBER  512
//
//  MP_ADAPTER flags
//
#define fRTMP_ADAPTER_TEST_MODE                0x00000001
#define fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS   0x00000002
#define fRTMP_ADAPTER_HARDWARE_ERROR           0x00000004

#define fRTMP_ADAPTER_SEND_PACKET_ERROR        0x00000010
#define fRTMP_ADAPTER_RECEIVE_PACKET_ERROR     0x00000020
#define fRTMP_ADAPTER_HALT_IN_PROGRESS         0x00000040
#define fRTMP_ADAPTER_RESET_IN_PROGRESS        0x00000080

#define fRTMP_ADAPTER_NIC_NOT_EXIST            0x00000100
#define fRTMP_ADAPTER_TX_RING_ALLOCATED        0x00000200
#define fRTMP_ADAPTER_ATIM_RING_ALLOCATED      0x00000400
#define fRTMP_ADAPTER_PRIO_RING_ALLOCATED      0x00000800

#define fRTMP_ADAPTER_RX_RING_ALLOCATED        0x00001000
#define fRTMP_ADAPTER_BSS_JOIN_IN_PROGRESS     0x00002000
#define fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS     0x00004000
#define fRTMP_ADAPTER_REASSOC_IN_PROGRESS      0x00008000

#define fRTMP_ADAPTER_MEDIA_STATE_PENDING      0x00010000
#define fRTMP_ADAPTER_RADIO_OFF                0x00020000
#define fRTMP_ADAPTER_BULKOUT_RESET            0x00040000
#define fRTMP_ADAPTER_BULKIN_RESET             0x00080000

#define fRTMP_ADAPTER_RDG_ACTIVE					0x00100000
//#define fRTMP_ADAPTER_RESET_DATA_SW_QUEUE      0x00100000
#define fRTMP_ADAPTER_RESET_PIPE_IN_PROGRESS   0x00200000
#define fRTMP_ADAPTER_IDLE_RADIO_OFF           0x00400000
#define fRTMP_ADAPTER_SUSPENDED                0x00800000

#define fRTMP_ADAPTER_SCAN_REQ_ACTION_IN_PROGRESS   0x01000000
//#define fRTMP_ADAPTER_SCAN_CHANNEL_IN_PROGRESS 0x04000000
#define fRTMP_ADAPTER_CONNECTED_AND_IN_P2pMsSESSION    0x04000000
#define fRTMP_ADAPTER_RADIO_MEASUREMENT        0x08000000

#define fRTMP_ADAPTER_SURPRISE_REMOVED         0x10000000
#define fRTMP_ADAPTER_MEDIA_STATE_CHANGE       0x20000000
#define fRTMP_ADAPTER_SLEEP_IN_PROGRESS        0x40000000
#define fRTMP_ADAPTER_ADDKEY_IN_PROGRESS       0x80000000

//
//  STA operation status flags
//
#define fOP_STATUS_INFRA_ON                                     0x00000001
#define fOP_STATUS_ADHOC_ON                                     0x00000002
#define fOP_STATUS_BG_PROTECTION_INUSED                         0x00000004
#define fOP_STATUS_SHORT_SLOT_INUSED                                0x00000008
#define fOP_STATUS_SHORT_PREAMBLE_INUSED                        0x00000010
#define fOP_STATUS_RECEIVE_DTIM                                 0x00000020
#define fOP_STATUS_TX_RATE_SWITCH_ENABLED                       0x00000040
#define fOP_STATUS_MEDIA_STATE_CONNECTED                        0x00000080
#define fOP_STATUS_WMM_INUSED                                   0x00000100
#define fOP_STATUS_AGGREGATION_INUSED                           0x00000200
#define fOP_STATUS_DOZE                                         0x00000400
#define fOP_STATUS_MAX_RETRY_ENABLED                                0x00000800
#define fOP_STATUS_RTS_PROTECTION_ENABLE                            0x00001000
#define fOP_STATUS_USECCK_PROTECTION_ENABLE                     0x00002000
//For MULTI_CHANNEL_SUPPORT
#define fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_SWITCHING      0x00004000
#define fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_WITH_NULLFRAME     0x00008000
#define fOP_STATUS_INFRA_OP                                     0x00010000
// For MULTI_CHANNEL_SUPPORT end

// SoftAP down status
#define fOP_STATUS_SOFTAP_IS_DOWN               0x00004000
#define fOP_STATUS_LONG_RETRY               0x00008000
#define fOP_STATUS_SCAN_2040               0x00010000

//
// Flags for Bulkflags control for bulk out data
//
#define fRTUSB_BULK_OUT_RTS                     0x00000002
#define fRTUSB_BULK_OUT_BEACON_1                0x00000008

#define fRTUSB_BULK_OUT_DATA_NORMAL             0x00010000
#define fRTUSB_BULK_OUT_DATA_NORMAL_2           0x00020000
#define fRTUSB_BULK_OUT_DATA_NORMAL_3           0x00040000
#define fRTUSB_BULK_OUT_DATA_NORMAL_4           0x00080000


#define fRTUSB_BULK_OUT_BEACON_0                0x00000010
#define fRTUSB_BULK_OUT_DATA_FRAG               0x00000040
#define fRTUSB_BULK_OUT_DATA_FRAG_2             0x00000080
#define fRTUSB_BULK_OUT_DATA_FRAG_3             0x00000100
#define fRTUSB_BULK_OUT_DATA_FRAG_4             0x00000200

#define CCKSETPROTECT       0x1
#define OFDMSETPROTECT      0x2
#define MM20SETPROTECT      0x4
#define MM40SETPROTECT      0x8
#define GF20SETPROTECT      0x10
#define GF40SETPROTECT      0x20
#define ALLN_SETPROTECT     (GF40SETPROTECT|GF20SETPROTECT|MM40SETPROTECT|MM20SETPROTECT)
#define ALLERP_SETPROTECT       (CCKSETPROTECT|OFDMSETPROTECT)

/** The default channel that the NIC will initialize itself with in b/g mode*/
#define HW_DEFAULT_CHANNEL        11

/** The default channel that the NIC will initialize itself with in "a" mode*/
#define HW_DEFAULT_CHANNEL_A      52

// 802.11n Operating Mode Definition. 0-3 also used in ASICUPdateProtect switch case 
#define HT_NO_PROTECT   0
#define HT_LEGACY_PROTECT   1
#define HT_40_PROTECT   2
#define HT_2040_PROTECT 3
#define HT_80_PROTECT   4
//following is our own definition in order to turn on our ASIC protection register in INFRASTRUCTURE.
#define HT_RTSCTS_6M    7
#define HT_ATHEROS  8   // rt2860c has problem with atheros chip. we need to turn on RTS/CTS .
#define HT_FORCERTSCTS  9   // Force turn on RTS/CTS first. then go to evaluate if this force RTS is necessary.

//
//  STA configuration flags
//
//#define fSTA_CFG_ENABLE_TX_BURST          0x00000001

//
// RX Packet Filter control flags. Apply on pAd->PacketFilter
//
#define fRX_FILTER_ACCEPT_DIRECT            NDIS_PACKET_TYPE_DIRECTED
#define fRX_FILTER_ACCEPT_MULTICAST         NDIS_PACKET_TYPE_MULTICAST
#define fRX_FILTER_ACCEPT_BROADCAST         NDIS_PACKET_TYPE_BROADCAST
#define fRX_FILTER_ACCEPT_ALL_MULTICAST     NDIS_PACKET_TYPE_ALL_MULTICAST
#define fRX_FILTER_ACCEPT_PROMISCUOUS       NDIS_PACKET_TYPE_PROMISCUOUS

//
// Error code section
//
// NDIS_ERROR_CODE_ADAPTER_NOT_FOUND
#define ERRLOG_READ_PCI_SLOT_FAILED     0x00000101L
#define ERRLOG_WRITE_PCI_SLOT_FAILED    0x00000102L
#define ERRLOG_VENDOR_DEVICE_NOMATCH    0x00000103L

// NDIS_ERROR_CODE_ADAPTER_DISABLED
#define ERRLOG_BUS_MASTER_DISABLED      0x00000201L

// NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION
#define ERRLOG_INVALID_SPEED_DUPLEX     0x00000301L
#define ERRLOG_SET_SECONDARY_FAILED     0x00000302L

// NDIS_ERROR_CODE_OUT_OF_RESOURCES
#define ERRLOG_OUT_OF_MEMORY            0x00000401L
#define ERRLOG_OUT_OF_SHARED_MEMORY     0x00000402L
#define ERRLOG_OUT_OF_MAP_REGISTERS     0x00000403L
#define ERRLOG_OUT_OF_BUFFER_POOL       0x00000404L
#define ERRLOG_OUT_OF_NDIS_BUFFER       0x00000405L
#define ERRLOG_OUT_OF_PACKET_POOL       0x00000406L
#define ERRLOG_OUT_OF_NDIS_PACKET       0x00000407L
#define ERRLOG_OUT_OF_LOOKASIDE_MEMORY  0x00000408L

// NDIS_ERROR_CODE_HARDWARE_FAILURE
#define ERRLOG_SELFTEST_FAILED          0x00000501L
#define ERRLOG_INITIALIZE_ADAPTER       0x00000502L
#define ERRLOG_REMOVE_MINIPORT          0x00000503L

// NDIS_ERROR_CODE_RESOURCE_CONFLICT
#define ERRLOG_MAP_IO_SPACE             0x00000601L
#define ERRLOG_QUERY_ADAPTER_RESOURCES  0x00000602L
#define ERRLOG_NO_IO_RESOURCE           0x00000603L
#define ERRLOG_NO_INTERRUPT_RESOURCE    0x00000604L
#define ERRLOG_NO_MEMORY_RESOURCE       0x00000605L

// definition to support multiple BSSID
#define BSS0                            0
#define BSS1                            1
#define BSS2                            2
#define BSS3                            3
#define MAX_NUM_OF_BSS                  4


//============================================================
// Length definitions
#define PEER_KEY_NO                     2
#define MAC_ADDR_LEN                    6
#define TIMESTAMP_LEN                   8
#define BEACON_INTVERVAL_LEN     2
#define CAPABILITY_LEN     2

#define MAX_LEN_OF_SUPPORTED_RATES      12    // 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54
#define MAX_LEN_OF_CCK_RATES            4       // 1, 2, 5.5, 11
#define MAX_LEN_OF_OFDM_RATES           8       // 6, 9, 12, 18, 24, 36, 48, 54
#define MAX_LEN_OF_KEY                  32      // 32 octets == 256 bits, Redefine for WPA
#define MAX_NUM_OF_CHANNELS             59      // 14 channels @2.4G +  12@UNII(lower/middle) + 16@HiperLAN2 + 11@UNII(upper) + 0@Japan + 1 as NULL termination
#define MAX_NUM_OF_11JCHANNELS             20      // 14 channels @2.4G +  12@UNII + 4 @MMAC + 11 @HiperLAN2 + 7 @Japan + 1 as NULL termination
#define MAX_LEN_OF_SSID                 32
#define CIPHER_TEXT_LEN                 128
#define HASH_TABLE_SIZE                 256
#define MAX_VIE_LEN                     1024   // New for WPA cipher suite variable IE sizes.
#define MAX_SUPPORT_MCS             32    
#define MS_WFD_BYPASS_LEN           (TIMESTAMP_LEN + BEACON_INTVERVAL_LEN + CAPABILITY_LEN)

//============================================================
// ASIC WCID Table definition.
//============================================================
#define HW_BEACON_MAX_NUM   16
#define MAX_APCLI_NUM   0

//#define AP_MBCAST_WCID  31  
//#define MBCAST_WCID 0    
#define BSSID_WCID      1   // in infra mode, always put bssid with this WCID 
//#define VWF_BSSID_WCID    2      // in Vertual Wifi, the WCID for STA starts from 2
//#define BT_WCID         8       // Assume the max 8 BT devices.
//#define P2P_CLIENT_WCID 23  // concurrent p2p client and win8-wfd-client #1
//#define P2P_CLIENT_WCID_2   22  // concurrent p2p client and win8-wfd-client #2
//#define MCAST_WCID  0
//#define APCLI_MCAST_WCID    (MAX_LEN_OF_MAC_TABLE + HW_BEACON_MAX_NUM + MAX_APCLI_NUM)
//#define BSS0Mcast_WCID  0x0  
//#define BSS1Mcast_WCID  (ULONG)((0xF8))
//#define BSS2Mcast_WCID  (ULONG)((0xF9))
//#define BSS3Mcast_WCID  (ULONG)((0xFA))
//#define BSS4Mcast_WCID  (ULONG)((0xFB))
//#define BSS5Mcast_WCID  (ULONG)((0xFC))
//#define BSS6Mcast_WCID  (ULONG)((0xFD))
//#define BSS7Mcast_WCID  (ULONG)((0xFE))
#define RESERVED_WCID   0xff
#define WCID_ALL              0xff
#define MLME_WCID       (ULONG)((0xFF))  //the ASIC WCID in 7601 only have 128!!!

#define MAX_LEN_OF_MAC_TABLE            128
#define MAX_NUM_OF_WDS_LINK             3
#define MAX_NUM_OF_EVENT                10  // entry # in EVENT table

#define NUM_OF_TID          8
#define MAX_LEN_OF_BA_REC_TABLE             (8)  //Block ACK recipient 
#define MAX_LEN_OF_BA_ORI_TABLE             (32)   // Block ACK originator
#define MAX_LEN_OF_BSS_TABLE             128

#define MAX_TX_POWER_LEVEL              100   /* mW */
#define MAX_RSSI_TRIGGER                -10    /* dBm */
#define MIN_RSSI_TRIGGER                -200   /* dBm */
#define MAX_FRAG_THRESHOLD              2346  /* byte count */
#define MIN_FRAG_THRESHOLD              256   /* byte count */
#define MAX_RTS_THRESHOLD               2347  /* byte count */

#define MAX_ROAMING_RETRY_COUNT               10

// power status related definitions
#define NORMAL_RECEIVE                      0   // Packet arrives in sequence OR not under BA session, ==> use normal receive method
#define PUT_TO_REORDER                        1 // This packet already put into reordering queue.
//#define DROP_DUPLICATE                        2           // this is duplicate packet
#define REORDER_FLUSH                        3          // flush 
#define REORDER_INVALID                        4            // flush 
#define REORDER_REFRESH_FLUSH                        5          // flush 

// key related definitions
#define SHARE_KEY_NUM                   4
#define MAX_LEN_OF_SHARE_KEY            16    // byte count
#define MAX_LEN_OF_PEER_KEY             16    // byte count
#define PAIRWISE_KEY_NUM                64    // in MAC ASIC pairwise key table
#define GROUP_KEY_NUM                   4
#define PMK_LEN                         32
#define WDS_PAIRWISE_KEY_OFFSET         60    // WDS links uses pairwise key#60 ~ 63 in ASIC pairwise key table
#define MIC_KEY_LEN                     8

// power status related definitions
#define PWR_ACTIVE                      0
#define PWR_SAVE                        1
#define PWR_MMPS                        2           //MIMO power save
//#define PWR_UNKNOWN                   3

// Auth and Assoc mode related definitions
#define AUTH_OPEN_SYSTEM                  0x00
#define AUTH_SHARED_KEY                   0x01
//#define AUTH_MODE_AUTO_SWITCH         0x03
//#define AUTH_MODE_DEAUTH              0x04
//#define AUTH_MODE_UPLAYER             0x05 // reserved for 802.11i use

// BSS Type definitions
#define BSS_ADHOC                       dot11_BSS_type_independent
#define BSS_INFRA                       dot11_BSS_type_infrastructure
#define BSS_ANY                         dot11_BSS_type_any
#define BSS_MONITOR	        BSS_ANY	/* = Ndis802_11Monitor */

// Reason code definitions
#define REASON_RESERVED                 0
#define REASON_UNSPECIFY                1
#define REASON_NO_LONGER_VALID          2
#define REASON_DEAUTH_STA_LEAVING       3
#define REASON_DISASSOC_INACTIVE        4
#define REASON_DISASSPC_AP_UNABLE       5
#define REASON_CLS2ERR                  6
#define REASON_CLS3ERR                  7
#define REASON_DISASSOC_STA_LEAVING     8
#define REASON_STA_REQ_ASSOC_NOT_AUTH   9
#define REASON_DISASSOC_POWER_CAP_UNACCEPTABLE  10
#define REASON_DISASSOC_CHANNEL_UNACCEPTABLE    11
#define REASON_INVALID_IE               13
#define REASON_MIC_FAILURE              14
#define REASON_4_WAY_TIMEOUT            15
#define REASON_GROUP_KEY_HS_TIMEOUT     16
#define REASON_IE_DIFFERENT             17
#define REASON_MCIPHER_NOT_VALID        18
#define REASON_UCIPHER_NOT_VALID        19
#define REASON_AKMP_NOT_VALID           20
#define REASON_UNSUPPORT_RSNE_VER       21
#define REASON_INVALID_RSNE_CAP         22
#define REASON_8021X_AUTH_FAIL          23
#define REASON_CIPHER_SUITE_REJECTED    24
#define REASON_FT_INVALID_FTIE      55
#define REASON_DECLINED                 37

#define REASON_ASSOC_REJ_TRY_LATER    30
#define REASON_RMF_POLICY_VIOLATION   31
#define REASON_P2P_PEER_MANUAL_DEAUTH       51 // only persistent on and GO send disconnect to Client use this special reason code(only maunal disconnect)

// Status code definitions
#define MLME_SUCCESS                    0
#define MLME_UNSPECIFY_FAIL             1
#define MLME_CANNOT_SUPPORT_CAP         10
#define MLME_REASSOC_DENY_ASSOC_EXIST   11 
#define MLME_ASSOC_DENY_OUT_SCOPE       12
#define MLME_ALG_NOT_SUPPORT            13
#define MLME_SEQ_NR_OUT_OF_SEQUENCE     14
#define MLME_REJ_CHALLENGE_FAILURE      15
#define MLME_REJ_TIMEOUT                  16
#define MLME_ASSOC_REJ_UNABLE_HANDLE_STA  17
#define MLME_ASSOC_REJ_DATA_RATE          18
#define MLME_NOT_SUPPORT_SHORT_PREAMBLE 19

#define MLME_ASSOC_REJ_NO_EXT_RATE        22
#define MLME_ASSOC_REJ_NO_EXT_RATE_PBCC   23
#define MLME_ASSOC_REJ_NO_CCK_OFDM        24
#define MLME_NOT_SUPPORT_DSSS_OFDM        25

#define MLME_ASSOC_REJ_TRY_LATER    30
#define MLME_RMF_POLICY_VIOLATION   31

#define MLME_INVALID_FORMAT             0x51
#define MLME_FAIL_NO_RESOURCE           0x52
#define MLME_STATE_MACHINE_REJECT       0x53
#define MLME_MAC_TABLE_FAIL             0x54

#define MLME_QOS_UNSPECIFY                32
#define MLME_DENY_QOSAP_INSUFFICIENT_BW 33
#define MLME_DENY_NOT_SUPPORT_QOS   35
#define MLME_REQUEST_DECLINED             37
#define MLME_REQUEST_WITH_INVALID_PARAM   38
#define MLME_INVALID_INFORMATION_ELEMENT 40
#define MLME_INVALID_AKMP           43
#define MLME_NOT_SUPPORT_RSN_VERSION    44
#define MLME_INVALID_RSN_CAPABILITIES       45
#define MLME_REJ_SECURITY_CIPHER_SUITE  46
#define MLME_DLS_NOT_ALLOW_IN_QBSS        48
#define MLME_DEST_STA_NOT_IN_QBSS         49
#define MLME_DEST_STA_IS_NOT_A_QSTA       50

// TDLS status code
#define MLME_WAKEUP_REJ_BUT_ALT_SCH_PRO         2
#define MLME_WAKEUP_REJ                 3
#define MLEM_TDLS_NOT_ALLOW_IN_BSS          4
#define MLME_SECURITY_DISABLED              5
#define MLME_UNACCEPTABLE_LIFETIME          6
#define MLME_NOT_IN_SAME_BSS                7


#define REASON_QOS_UNSPECIFY              32    
#define REASON_QOS_LACK_BANDWIDTH         33    
#define REASON_POOR_CHANNEL_CONDITION     34
#define REASON_QOS_OUTSIDE_TXOP_LIMITION  35
#define REASON_QOS_QSTA_LEAVING_QBSS      36
#define REASON_QOS_UNWANTED_MECHANISM     37
#define REASON_QOS_MECH_SETUP_REQUIRED    38
#define REASON_QOS_REQUEST_TIMEOUT        39
#define REASON_QOS_CIPHER_NOT_SUPPORT     45

// TDLS reason code
#define REASON_TDLS_UNACCEPTABLE_FRAME_LOSS 25
#define REASON_TDLS_UNSPECIFY           26


// IE code
#define IE_SSID                         0
#define IE_SUPP_RATES                   1
#define IE_FH_PARM                      2
#define IE_DS_PARM                      3
#define IE_CF_PARM                      4
#define IE_TIM                          5
#define IE_IBSS_PARM                    6
#define IE_COUNTRY                      7     // 802.11d
#define IE_802_11D_REQUEST              10    // 802.11d
#define IE_QBSS_LOAD                    11    // 802.11e d9
#define IE_EDCA_PARAMETER               12    // 802.11e d9
#define IE_TSPEC                        13    // 802.11e d9 
#define IE_TCLAS                        14    // 802.11e d9
#define IE_SCHEDULE                     15    // 802.11e d9
#define IE_CHALLENGE_TEXT               16
#define IE_POWER_CONSTRAINT             32    // 802.11h d3.3
#define IE_POWER_CAPABILITY             33    // 802.11h d3.3
#define IE_TPC_REQUEST                  34    // 802.11h d3.3
#define IE_TPC_REPORT                   35    // 802.11h d3.3
#define IE_SUPP_CHANNELS                36    // 802.11h d3.3
#define IE_CHANNEL_SWITCH_ANNOUNCEMENT  37    // 802.11h d3.3
#define IE_MEASUREMENT_REQUEST          38    // 802.11h d3.3
#define IE_MEASUREMENT_REPORT           39    // 802.11h d3.3
#define IE_QUIET                        40    // 802.11h d3.3
#define IE_IBSS_DFS                     41    // 802.11h d3.3
#define IE_ERP                          42    // 802.11g
#define IE_TS_DELAY                     43    // 802.11e d9
#define IE_TCLAS_PROCESSING             44    // 802.11e d9
#define IE_QOS_CAPABILITY               46    // 802.11e d6
#define IE_HT_CAP                         45    // 802.11n d1. HT CAPABILITY. ELEMENT ID TBD
#define IE_HT_CAP2                         52    // 802.11n d1. HT CAPABILITY. ELEMENT ID TBD
#define IE_RSN                          48    // 802.11i d3.0
#define IE_WPA2                         48    // WPA2
#define IE_EXT_SUPP_RATES               50    // 802.11g
#define IE_FAST_BSS_TRANSITION               55    // 802.11R
#define IE_TIMEOUT_INTERVAL              56    // 802.11R
#define IE_SUPP_REG_CLASS               59    // 802.11y. Supported regulatory classes.
#define IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT  60    // 802.11y
#define IE_ADD_HT                         61    // 802.11n d1. ADDITIONAL HT CAPABILITY. ELEMENT ID TBD
#define IE_ADD_HT2                        53    // 802.11n d1. ADDITIONAL HT CAPABILITY. ELEMENT ID TBD
#define IE_NEW_EXT_CHA_OFFSET                         62    // 802.11n d1. New extension channel offset elemet
#define IE_WPA                          221   // WPA
#define IE_VENDOR_SPECIFIC              221   // Wifi WMM (WME)
#define IE_WFA_WSC                      221
#define OUI_PREN_HT_CAP              51   //  
#define OUI_PREN_ADD_HT              52   //  
#define IE_WAPI                  68
#define IE_CHANNEL_USAGE                97  // Cisco advertises suggested channel using this IE.

// For 802.11z D6.0
#define IE_TDLS_LINK_ID         101 // 802.11z

// For 802.11n D3.03
#define IE_NEW_EXT_CHA_OFFSET                         62    // 802.11n d1. New extension channel offset elemet
#define IE_2040_BSS_COEXIST                         72    // 802.11n D3.0.3
#define IE_2040_BSS_INTOLERANT_REPORT                        73    // 802.11n D3.03
#define IE_OVERLAPBSS_SCAN_PARM                         74    // 802.11n D3.03

#define IE_MANAGEMENT_MIC               76      // 802.11w D9.0 MMIE for BIP

#define IE_EXT_CAPABILITY                          127   // 802.11n D3.03 


// ========================================================
// MLME state machine definition
// ========================================================

// STA MLME state mahcines
#define ASSOC_STATE_MACHINE             1
#define AUTH_STATE_MACHINE              2
#define AUTH_RSP_STATE_MACHINE          3
#define SYNC_STATE_MACHINE              4
#define MLME_CNTL_STATE_MACHINE         5
#define WPA_PSK_STATE_MACHINE           6
#define DLS_STATE_MACHINE               7
#define ACTION_STATE_MACHINE            8
#define WSC_STATE_MACHINE               9

// AP MLME state machines
#define AP_ASSOC_STATE_MACHINE          10
#define AP_AUTH_STATE_MACHINE           11
#define AP_AUTH_RSP_STATE_MACHINE       12
#define AP_SYNC_STATE_MACHINE           13
#define AP_CNTL_STATE_MACHINE           14
#define AP_WPA_STATE_MACHINE            15

// TDLS MLME state machine
#define TDLS_STATE_MACHINE              16

#define P2P_STATE_MACHINE               17

// MFP MLME state machines
#define STA_PMF_STATE_MACHINE           18

//
// STA's CONTROL/CONNECT state machine: states, events, total function #
//
#define CNTL_IDLE                       0
#define CNTL_WAIT_DISASSOC              1
#define CNTL_WAIT_JOIN                  2
#define CNTL_WAIT_REASSOC               3
#define CNTL_WAIT_START                 4
#define CNTL_WAIT_AUTH                  5
#define CNTL_WAIT_ASSOC                 6
#define CNTL_WAIT_AUTH2                 7
#define CNTL_WAIT_OID_LIST_SCAN         8
#define CNTL_WAIT_OID_DISASSOC          9
#define CNTL_WAIT_SCAN_FOR_CONNECT      10

#define MT2_ASSOC_CONF                  34
#define MT2_AUTH_CONF                   35
#define MT2_DEAUTH_CONF                 36
#define MT2_DISASSOC_CONF               37
#define MT2_REASSOC_CONF                38
#define MT2_PWR_MGMT_CONF               39
#define MT2_JOIN_CONF                   40
#define MT2_SCAN_CONF                   41
#define MT2_START_CONF                  42
#define MT2_GET_CONF                    43
#define MT2_SET_CONF                    44
#define MT2_RESET_CONF                  45
#define MT2_MLME_ROAMING_REQ            52

#define CNTL_FUNC_SIZE                  1

//
// BTHS state machine
//
#define BT_STATE_MACHINE_COUNT      8
#define BT_DEFAULT_ONE              0   // temporal usage

// BTHS State Machine : State

#define BT_DISCONNECTED 0
#define BT_STARTING         1
#define BT_CONNECTING       2
#define BT_AUTHENTICATING   3
#define BT_CONNECTED        4
#define BT_DISCONNECTING    5
#define BT_MAX_STATE        6

// BTHS State Machine : Event

#define BT_MACHINE_BASE             0
#define HCI_Create_Physical_Link            0
#define HCI_Accept_Physical_Link            1
#define HCI_Disconnect_Physical_Link        2
#define Connection_Accept_Timeout       3
#define MAC_Start_Completed             4
#define MAC_Start_Failed                    5
#define MAC_Connect_Completed           6
#define MAC_Connect_Failed              7
#define MAC_Media_Disconnection_Ind     8
#define MAC_Connection_Cancel_Ind       9
#define Four_Way_Handshake_Fail     10
#define Four_Way_Handshake_Success  11
#define BT_MAX_MSG                      12

#define BT_FUNC_SIZE        (BT_MAX_STATE*BT_MAX_MSG)

//
// BTHS's CONTROL/CONNECT state machine: states, events, total function #
//
#define BT_CNTL_IDLE                       0
//#define BT_CNTL_WAIT_DISASSOC              1
#define BT_CNTL_WAIT_JOIN                  1
//#define BT_CNTL_WAIT_REASSOC               3
//#define BT_CNTL_WAIT_START                 4
#define BT_CNTL_WAIT_AUTH                  2
#define BT_CNTL_WAIT_ASSOC                 3
//#define BT_CNTL_WAIT_AUTH2                 7
//#define BT_CNTL_WAIT_ASSOC2                 8
//#define BT_CNTL_WAIT_OID_LIST_SCAN         9
//#define BT_CNTL_WAIT_OID_DISASSOC          10
//#define BT_CNTL_WAIT_SCAN_FOR_CONNECT     11

//
// BTHS ASSOC state machine: states, events, total function #
//
#define BT_ASSOC_IDLE                      0
#define BT_ASSOC_WAIT_RSP                  1
//#define REASSOC_WAIT_RSP                2
//#define DISASSOC_WAIT_RSP               3
#define BT_MAX_ASSOC_STATE                 2

#define BT_ASSOC_MACHINE_BASE              0
#define BT_MT2_MLME_ASSOC_REQ              0
//#define MT2_MLME_REASSOC_REQ            1
#define BT_MT2_MLME_DISASSOC_REQ           1  
#define BT_MT2_PEER_DISASSOC_REQ           2
#define BT_MT2_PEER_ASSOC_REQ              3
#define BT_MT2_PEER_ASSOC_RSP              4
//#define MT2_PEER_REASSOC_REQ            6
//#define MT2_PEER_REASSOC_RSP            7
//#define MT2_DISASSOC_TIMEOUT            8
#define BT_MT2_ASSOC_TIMEOUT               5
//#define MT2_REASSOC_TIMEOUT             10
#define BT_MAX_ASSOC_MSG                   6

#define BT_ASSOC_FUNC_SIZE                 (BT_MAX_ASSOC_STATE * BT_MAX_ASSOC_MSG)

//
// BTHS's AUTHENTICATION state machine: states, evvents, total function #
//
#define BT_AUTH_REQ_IDLE                   0
#define BT_AUTH_WAIT_SEQ2                  1
//#define AUTH_WAIT_SEQ4                  2
#define BT_MAX_AUTH_STATE                  2

#define BT_AUTH_MACHINE_BASE               0
#define BT_MT2_MLME_AUTH_REQ               0
#define BT_MT2_PEER_AUTH_EVEN              1
#define BT_MT2_PEER_AUTH_ODD                2
#define BT_MT2_AUTH_TIMEOUT                3
#define BT_MAX_AUTH_MSG                    4

#define BT_AUTH_FUNC_SIZE                  (BT_MAX_AUTH_STATE * BT_MAX_AUTH_MSG)

//
// BTHS's SYNC state machine: states, events, total function #
//
#define BT_SYNC_IDLE                       0  // merge NO_BSS,IBSS_IDLE,IBSS_ACTIVE and BSS in to 1 state
#define BT_JOIN_WAIT_BEACON                1
//#define SCAN_LISTEN                     2
//#define SCAN_PENDING                    3
#define BT_MAX_SYNC_STATE                  2

#define BT_SYNC_MACHINE_BASE               0
//#define MT2_MLME_SCAN_REQ               0
#define BT_MT2_MLME_JOIN_REQ               0
//#define MT2_MLME_START_REQ              2
#define BT_MT2_PEER_BEACON                 1
#define BT_MT2_PEER_PROBE_RSP              2
//#define MT2_PEER_ATIM                   5
//#define MT2_SCAN_TIMEOUT                6
#define BT_MT2_BEACON_TIMEOUT              3
//#define MT2_ATIM_TIMEOUT                8
#define BT_MT2_PEER_PROBE_REQ              4
#define BT_MAX_SYNC_MSG                    5

#define BT_SYNC_FUNC_SIZE                  (BT_MAX_SYNC_STATE * BT_MAX_SYNC_MSG)

//
// BTHS's WPA2PSK state machine: states, events, total function #
//
#define BT_WPA_PTK                      0
#define BT_MAX_WPA_PTK_STATE            1

#define BT_WPA_MACHINE_BASE             0
//#define BTMT2_EAPPacket                 0
#define BTMT2_EAPOLStart                0
//#define BTMT2_EAPOLLogoff               2
#define BTMT2_EAPOLKey                  1
//#define BTMT2_EAPOLASFAlert             4
#define BT_MAX_WPA_MSG                  2

#define BT_WPA_FUNC_SIZE                (BT_MAX_WPA_PTK_STATE * BT_MAX_WPA_MSG)

// BTHS Status message
#define BT_MSG_SuccessNoErr 0x00    // Success without Error
#define BT_MSG_UnknowHCICmd 0x01    // Unknown HCI Command
#define BT_MSG_UnknowConnID 0x02    // Unknown connection identifier
#define BT_MSG_HARDWAREFAIL 0x03    // Hardware failure
#define BT_MSG_AuthFailed       0x05    // Authentication failure
#define BT_MSG_MEMCAPEXCED      0x07    // Memory Capacity Exceeded
#define BT_MSG_ConnTimeOut      0x08    // Connection time-out
#define BT_MSG_ConnLmtExcd      0x09    // Connection Limit Exceeded
#define BT_MSG_ACLCONNEXIST     0x0B    // ACL Connection already exists
#define BT_MSG_CMDDisallowed    0x0C    // Command disallowed
#define BT_MSG_RejectLmtRsc     0x0D    // Connection rejected due to limited resources
#define BT_MSG_CATExceeded      0x10    // Connection Accept Timeout Exceeded
#define BT_MSG_InvalidPrmt      0x12    // Invalid HCI Command Parameters
#define BT_MSG_ConnClzByRU      0x13    // Remote User Terminated Connection
#define BT_MSG_ConnClzByLH      0x16    // Connection terminated by local host
#define BT_MSG_QOSREJECTED      0x2D    // Qos Rejected
#define BT_MSG_ControlBusy      0x3A    // Controller Busy
#define BT_MSG_CONNCLZBYLH      0x16    // Connection terminated by local host
#define BT_MSG_CONTROLBUSY      0x3A    // Controller Busy
#define BT_MSG_CONNRJTNOCH      0x39    // Connection Rejected due to No Suitable Channel Found

// BTHS OP Code define

// OGF
#define BT_HCI_OGF_LINK_CONTROL     0x01
#define BT_HCI_OGF_CONTROLLER_BB        0x03
#define BT_HCI_OGF_INFORM_PARAM     0x04
#define BT_HCI_OGF_STATUS_PARAM     0x05
#define BT_HCI_OGF_TESTING_CMD      0x06

// OCF
// For OGF == 0x01
#define BT_HCI_OCF_CREATE_PHYSICAL_LINK         0x0035
#define BT_HCI_OCF_ACCEPT_PHYSICAL_LINK         0x0036
#define BT_HCI_OCF_DISCONNECT_PHYSICAL_LINK     0x0037
#define BT_HCI_OCF_CREATE_LOGICAL_LINK          0x0038
#define BT_HCI_OCF_ACCEPT_LOGICAL_LINK          0x0039
#define BT_HCI_OCF_DISCONNECT_LOGICAL_LINK      0x003A
#define BT_HCI_OCF_LOGICAL_LINK_CANCEL          0x003B
#define BT_HCI_OCF_FLOW_SPEC_MODIFY             0x003C

// For OGF == 0x03
#define BT_HCI_OCF_SET_EVENT_MASK               0x0001
#define BT_HCI_OCF_RESET                            0x0003
#define BT_HCI_OCF_SET_EVENT_FILTER             0x0005
#define BT_HCI_OCF_READ_CONN_ACPT_TIMEOUT       0x0015
#define BT_HCI_OCF_WRITE_CONN_ACPT_TIMEOUT      0x0016
#define BT_HCI_OCF_HOST_NUM_CMPD_PACKETS        0x0035
#define BT_HCI_OCF_READ_LINK_SV_TIMEOUT         0x0036
#define BT_HCI_OCF_WRITE_LINK_SV_TIMEOUT            0x0037
#define BT_HCI_OCF_ENHANCED_FLUSH               0x005F
#define BT_HCI_OCF_READ_LLINK_ACPT_TIMEOUT      0x0061
#define BT_HCI_OCF_WRITE_LLINK_ACPT_TIMEOUT     0x0062
#define BT_HCI_OCF_SET_EVENT_MASK_PAGE2         0x0063
#define BT_HCI_OCF_READ_LOCATION_DATA           0x0064
#define BT_HCI_OCF_WRITE_LOCATION_DATA          0x0065
#define BT_HCI_OCF_READ_FLOW_CTL_MODE           0x0066
#define BT_HCI_OCF_WRITE_FLOW_CTL_MODE          0x0067
#define BT_HCI_OCF_READ_BE_FLUSH_TIMEOUT        0x0069
#define BT_HCI_OCF_WRITE_BE_FLUSH_TIMEOUT       0x006A
#define BT_HCI_OCF_SHORT_RANGE_MODE             0x006B

// For OGF == 0x04
#define BT_HCI_OCF_READ_LOCAL_VERSION_INFO      0x0001
#define BT_HCI_OCF_READ_LOCAL_SP_CMD                0x0002
#define BT_HCI_OCF_READ_LOCAL_SP_FEATURE        0x0003
#define BT_HCI_OCF_READ_BUF_SIZE                    0x0005
#define BT_HCI_OCF_READ_DATA_BLOCK_SIZE         0x000A

// For OGF == 0x05
#define BT_HCI_OCF_READ_FAILED_CONTACT_CNT      0x0001
#define BT_HCI_OCF_RESET_FAILED_CONTACT_CNT     0x0002
#define BT_HCI_OCF_READ_LINK_QUALITY                0x0003
#define BT_HCI_OCF_READ_RSSI                        0x0005
#define BT_HCI_OCF_READ_LOCAL_AMP_INFO          0x0009
#define BT_HCI_OCF_READ_LOCAL_AMP_ASSOC         0x000A
#define BT_HCI_OCF_WRITE_REMOTE_AMP_ASSOC       0x000B

// For OGF == 0x06
#define BT_HCI_OCF_ENABLE_DEVICE_UNDER_TM       0x0003
#define BT_HCI_OCF_ENABLE_AMP_RCV_REPORT        0x0007
#define BT_HCI_OCF_AMP_TEST_END                 0x0008
#define BT_HCI_OCF_AMP_TEST_CMD                 0x0009

// BTHS HCI command
#define BT_HCI_CMD(nOCF, nOGF) ((unsigned short) nOCF + (unsigned short)(nOGF << 10))

// Link Control (8)
#define BT_HCI_CREATE_PHYSICAL_LINK     BT_HCI_CMD(BT_HCI_OCF_CREATE_PHYSICAL_LINK,     BT_HCI_OGF_LINK_CONTROL)
#define BT_HCI_ACCEPT_PHYSICAL_LINK     BT_HCI_CMD(BT_HCI_OCF_ACCEPT_PHYSICAL_LINK,     BT_HCI_OGF_LINK_CONTROL)
#define BT_HCI_DISCONNECT_PHYSICAL_LINK BT_HCI_CMD(BT_HCI_OCF_DISCONNECT_PHYSICAL_LINK, BT_HCI_OGF_LINK_CONTROL)
#define BT_HCI_CREATE_LOGICAL_LINK      BT_HCI_CMD(BT_HCI_OCF_CREATE_LOGICAL_LINK,      BT_HCI_OGF_LINK_CONTROL)
#define BT_HCI_ACCEPT_LOGICAL_LINK      BT_HCI_CMD(BT_HCI_OCF_ACCEPT_LOGICAL_LINK,      BT_HCI_OGF_LINK_CONTROL)
#define BT_HCI_DISCONNECT_LOGICAL_LINK  BT_HCI_CMD(BT_HCI_OCF_DISCONNECT_LOGICAL_LINK,  BT_HCI_OGF_LINK_CONTROL)
#define BT_HCI_LOGICAL_LINK_CANCEL      BT_HCI_CMD(BT_HCI_OCF_LOGICAL_LINK_CANCEL,      BT_HCI_OGF_LINK_CONTROL)
#define BT_HCI_FLOW_SPEC_MODIFY         BT_HCI_CMD(BT_HCI_OCF_FLOW_SPEC_MODIFY,         BT_HCI_OGF_LINK_CONTROL)

// Controller & Baseband (19)
#define BT_HCI_SET_EVENT_MASK               BT_HCI_CMD(BT_HCI_OCF_SET_EVENT_MASK,               BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_RESET                        BT_HCI_CMD(BT_HCI_OCF_RESET,                        BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_SET_EVENT_FILTER         BT_HCI_CMD(BT_HCI_OCF_SET_EVENT_FILTER,         BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_READ_CONN_ACPT_TIMEOUT   BT_HCI_CMD(BT_HCI_OCF_READ_CONN_ACPT_TIMEOUT,   BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_WRITE_CONN_ACPT_TIMEOUT  BT_HCI_CMD(BT_HCI_OCF_WRITE_CONN_ACPT_TIMEOUT,  BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_HOST_NUM_CMPD_PACKETS    BT_HCI_CMD(BT_HCI_OCF_HOST_NUM_CMPD_PACKETS,    BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_READ_LINK_SV_TIMEOUT     BT_HCI_CMD(BT_HCI_OCF_READ_LINK_SV_TIMEOUT,     BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_WRITE_LINK_SV_TIMEOUT        BT_HCI_CMD(BT_HCI_OCF_WRITE_LINK_SV_TIMEOUT,        BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_ENHANCED_FLUSH               BT_HCI_CMD(BT_HCI_OCF_ENHANCED_FLUSH,           BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_READ_LLINK_ACPT_TIMEOUT  BT_HCI_CMD(BT_HCI_OCF_READ_LLINK_ACPT_TIMEOUT,  BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_WRITE_LLINK_ACPT_TIMEOUT BT_HCI_CMD(BT_HCI_OCF_WRITE_LLINK_ACPT_TIMEOUT, BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_SET_EVENT_MASK_PAGE2     BT_HCI_CMD(BT_HCI_OCF_SET_EVENT_MASK_PAGE2,     BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_READ_LOCATION_DATA       BT_HCI_CMD(BT_HCI_OCF_READ_LOCATION_DATA,       BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_WRITE_LOCATION_DATA      BT_HCI_CMD(BT_HCI_OCF_WRITE_LOCATION_DATA,      BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_READ_FLOW_CTL_MODE       BT_HCI_CMD(BT_HCI_OCF_READ_FLOW_CTL_MODE,       BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_WRITE_FLOW_CTL_MODE      BT_HCI_CMD(BT_HCI_OCF_WRITE_FLOW_CTL_MODE,      BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_READ_BE_FLUSH_TIMEOUT        BT_HCI_CMD(BT_HCI_OCF_READ_BE_FLUSH_TIMEOUT,        BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_WRITE_BE_FLUSH_TIMEOUT   BT_HCI_CMD(BT_HCI_OCF_WRITE_BE_FLUSH_TIMEOUT,       BT_HCI_OGF_CONTROLLER_BB)
#define BT_HCI_SHORT_RANGE_MODE         BT_HCI_CMD(BT_HCI_OCF_SHORT_RANGE_MODE,         BT_HCI_OGF_CONTROLLER_BB)

// Informational Parameters (5)
#define BT_HCI_READ_LOCAL_VERSION_INFO  BT_HCI_CMD(BT_HCI_OCF_READ_LOCAL_VERSION_INFO,  BT_HCI_OGF_INFORM_PARAM)
#define BT_HCI_READ_LOCAL_SP_CMD            BT_HCI_CMD(BT_HCI_OCF_READ_LOCAL_SP_CMD,            BT_HCI_OGF_INFORM_PARAM)
#define BT_HCI_READ_LOCAL_SP_FEATURE        BT_HCI_CMD(BT_HCI_OCF_READ_LOCAL_SP_FEATURE,        BT_HCI_OGF_INFORM_PARAM)
#define BT_HCI_READ_BUF_SIZE                BT_HCI_CMD(BT_HCI_OCF_READ_BUF_SIZE,                BT_HCI_OGF_INFORM_PARAM)
#define BT_HCI_READ_DATA_BLOCK_SIZE     BT_HCI_CMD(BT_HCI_OCF_READ_DATA_BLOCK_SIZE,     BT_HCI_OGF_INFORM_PARAM)

// Status Parameters (7)
#define BT_HCI_READ_FAILED_CONTACT_CNT  BT_HCI_CMD(BT_HCI_OCF_READ_FAILED_CONTACT_CNT,  BT_HCI_OGF_STATUS_PARAM)
#define BT_HCI_RESET_FAILED_CONTACT_CNT BT_HCI_CMD(BT_HCI_OCF_RESET_FAILED_CONTACT_CNT, BT_HCI_OGF_STATUS_PARAM)
#define BT_HCI_READ_LINK_QUALITY            BT_HCI_CMD(BT_HCI_OCF_READ_LINK_QUALITY,            BT_HCI_OGF_STATUS_PARAM)
#define BT_HCI_READ_RSSI                    BT_HCI_CMD(BT_HCI_OCF_READ_RSSI,                    BT_HCI_OGF_STATUS_PARAM)
#define BT_HCI_READ_LOCAL_AMP_INFO      BT_HCI_CMD(BT_HCI_OCF_READ_LOCAL_AMP_INFO,      BT_HCI_OGF_STATUS_PARAM)
#define BT_HCI_READ_LOCAL_AMP_ASSOC     BT_HCI_CMD(BT_HCI_OCF_READ_LOCAL_AMP_ASSOC,     BT_HCI_OGF_STATUS_PARAM)
#define BT_HCI_WRITE_REMOTE_AMP_ASSOC   BT_HCI_CMD(BT_HCI_OCF_WRITE_REMOTE_AMP_ASSOC,   BT_HCI_OGF_STATUS_PARAM)

// Testing commands (4)
#define BT_HCI_ENABLE_DEVICE_UNDER_TM   BT_HCI_CMD(BT_HCI_OCF_ENABLE_DEVICE_UNDER_TM,   BT_HCI_OGF_TESTING_CMD)
#define BT_HCI_ENABLE_AMP_RCV_REPORT        BT_HCI_CMD(BT_HCI_OCF_ENABLE_AMP_RCV_REPORT,        BT_HCI_OGF_TESTING_CMD)
#define BT_HCI_AMP_TEST_END             BT_HCI_CMD(BT_HCI_OCF_AMP_TEST_END,             BT_HCI_OGF_TESTING_CMD)
#define BT_HCI_AMP_TEST_CMD             BT_HCI_CMD(BT_HCI_OCF_AMP_TEST_CMD,             BT_HCI_OGF_TESTING_CMD)


#define BT_HCI_CMD_MIN_SIZE     3
#define BT_HCI_CMD_MAX_SIZE     258

// BTHS 802.11 AMP TLVs TypeID
#define BT_TYPE_ID_MAC_ADDR     0x01
#define BT_TYPE_ID_PREFER_CH_LIST   0x02
#define BT_TYPE_ID_CONNECTED_CH 0x03
#define BT_TYPE_ID_PAL_CAP_LSIT 0x04
#define BT_TYPE_ID_PAL_VERSION      0x05

// BTHS HCI Event Code
#define BT_EVENT_CODE_COMMAND_COMPLETE                          0x0E
#define BT_EVENT_CODE_COMMAND_STATUS                            0x0F
#define BT_EVENT_CODE_HARDWARE_ERROR                            0x10
#define BT_EVENT_CODE_FLUSH_OCCURRED                                0x11
#define BT_EVENT_CODE_NUMBER_OF_COMPLETED_PACKETS               0x13
#define BT_EVENT_CODE_DATA_BUFFER_OVERFLOW                      0x1A
#define BT_EVENT_CODE_QOS_VIOLATION                             0x1E
#define BT_EVENT_CODE_ENHANCED_FLUSH_COMPLETE                   0x39
#define BT_EVENT_CODE_PHYSICAL_LINK_COMPLETE                    0x40
#define BT_EVENT_CODE_CHANNEL_SELECTED                          0x41
#define BT_EVENT_CODE_DISCONNECTION_PHYSICAL_LINK_COMPLETE  0x42
#define BT_EVENT_CODE_PHYSICAL_LINK_LOSS_EARLY_WARNING      0x43
#define BT_EVENT_CODE_PHYSICAL_LINK_RECOVERY                    0x44
#define BT_EVENT_CODE_LOGICAL_LINK_COMPLETE                     0x45
#define BT_EVENT_CODE_DISCONNECTION_LOGICAL_LINK_COMPLETE       0x46
#define BT_EVENT_CODE_FLOW_SPEC_MODIFY_COMPLETE             0x47
#define BT_EVENT_CODE_NUMBER_OF_COMPLETED_DATA_BLOCKS       0x48
#define BT_EVENT_CODE_AMP_START_TEST                                0x49
#define BT_EVENT_CODE_AMP_TEST_END                              0x4A
#define BT_EVENT_CODE_AMP_RECEIVER_REPORT                       0x4B
#define BT_EVENT_CODE_SHORT_RANGE_MODE_CHANGE_COMPLETE      0x4C
#define BT_EVENT_CODE_AMP_STATUS_CHANGE                     0x4D

// BTHS Data Flow Control Mode
#define BT_PacketBased      0x00
#define BT_DataBlockBased   0x01

// BTHS Physical Link Loss Early warning 
#define BT_PL_REASON_Unknown            0
#define BT_PL_REASON_Range_related      1
#define BT_PL_REASON_Bandwidth_related  2
#define BT_PL_REASON_Resolving_conflict 3
#define BT_PL_REASON_Interference       4

// BTHS AMP Status
#define BT_AMP_Status_PWR_Down      0x00
#define BT_AMP_Status_BTOnly            0x01
#define BT_AMP_Status_No_Capacity       0x02
#define BT_AMP_Status_Low_Capacity      0x03
#define BT_AMP_Status_Mid_Capacity      0x04
#define BT_AMP_Status_High_Capacity     0x05
#define BT_AMP_Status_Full_Capacity     0x06

// BTHS Event Mask
#define BT_EM_HardwareError             0x0000000000008000
#define BT_EM_FlushOccurred             0x0000000000010000
#define BT_EM_DataBufOverflow           0x0000000002000000
#define BT_EM_QosViolation              0x0000000020000000
#define BT_EM_EnhFlushCmp               0x0100000000000000

// BTHS Event Mask Page2
#define BT_EM_PhyLinkCmp                0x0000000000000001
#define BT_EM_ChSelected                0x0000000000000002
#define BT_EM_DiscPhyLinkCmp            0x0000000000000004
#define BT_EM_LogicalLinkCmp            0x0000000000000020
#define BT_EM_DiscLogicalLinkCmp        0x0000000000000040
#define BT_EM_FlowSpecModifyCmp     0x0000000000000080
#define BT_EM_NumCmpDataBlocks      0x0000000000000100
#define BT_EM_AMPStartTest              0x0000000000000200
#define BT_EM_AMPTestEnd                0x0000000000000400
#define BT_EM_AMPRecoverReport          0x0000000000000800
#define BT_EM_ShtRngModeChgCmp      0x0000000000001000
#define BT_EM_AMPStatusChg              0x0000000000002000

// BTHS Short Rage Mode status
#define BTHS_SRM_DISABLE                0x00
#define BTHS_SRM_ENABLE             0x01

// BTHS Service Type
#define BTHS_ST_NOTRAFFIC               0x00
#define BTHS_ST_BESTEFFORT          0x01
#define BTHS_ST_GUARANTEED          0x02

// BTHS AMP Status
#define BTHS_AMP_STATUS_00          0x00    //AMP radio is available , but needs to be power up
#define BTHS_AMP_STATUS_01          0x01    //AMP Controller is only used by Bluetooth tech
#define BTHS_AMP_STATUS_02          0x02    //AMP Controller has no capacity for Bluetooth operation
#define BTHS_AMP_STATUS_03          0x03    //AMP has LOW capacity available for Bluetooth
#define BTHS_AMP_STATUS_04          0x04    //AMP has MEDIUM capacity available for Bluetooth
#define BTHS_AMP_STATUS_05          0x05    //AMP has HIGH capacity available for Bluetooth
#define BTHS_AMP_STATUS_06          0x06    //AMP has FULL capacity available for Bluetooth

// definition of Bluetooth Mode
#define BTMODE_NONE         0
#define BTMODE_INITIATOR        1
#define BTMODE_RESPONDER        2


#define IOCTL_HCI_WRITE \
    CTL_CODE (FILE_DEVICE_NETWORK, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HCI_READ \
    CTL_CODE (FILE_DEVICE_NETWORK, 0x800, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

//
// STA's ASSOC state machine: states, events, total function #
//
#define ASSOC_IDLE                      0
#define ASSOC_WAIT_RSP                  1
#define REASSOC_WAIT_RSP                2
#define DISASSOC_WAIT_RSP               3
#define MAX_ASSOC_STATE                 4

#define ASSOC_MACHINE_BASE              0
#define MT2_MLME_ASSOC_REQ              0
#define MT2_MLME_REASSOC_REQ            1
#define MT2_MLME_DISASSOC_REQ           2  
#define MT2_PEER_DISASSOC_REQ           3
#define MT2_PEER_ASSOC_REQ              4
#define MT2_PEER_ASSOC_RSP              5
#define MT2_PEER_REASSOC_REQ            6
#define MT2_PEER_REASSOC_RSP            7
#define MT2_DISASSOC_TIMEOUT            8
#define MT2_ASSOC_TIMEOUT               9
#define MT2_REASSOC_TIMEOUT             10
#define MAX_ASSOC_MSG                   11

#define ASSOC_FUNC_SIZE                 (MAX_ASSOC_STATE * MAX_ASSOC_MSG)

//
// P2P state machine: states, events, total function #
//
#define P2P_IDLE_STATE              0
#define MAX_P2P_STATE               1

#define P2P_MACHINE_BASE            0
#define MT2_PEER_P2P_NOA            0
#define MT2_PEER_P2P_PRESENCE_REQ   1
#define MT2_PEER_P2P_PRESENCE_RSP   2
#define MT2_PEER_P2P_GO_DIS_REQ     3
#define MT2_MAX_PEER_SUPPORT        3
#define MT2_MLME_P2P_NOA            4
#define MT2_MLME_P2P_PRESENCE_REQ   5
#define MT2_MLME_P2P_PRESENCE_RSP   6
#define MT2_MLME_P2P_GO_DIS_REQ     7
#define MT2_MLME_P2P_GAS_INT_REQ    8
#define MT2_MLME_P2P_GAS_INT_RSP    9
#define MT2_MLME_P2P_GAS_CB_REQ     10
#define MT2_MLME_P2P_GAS_CB_RSP     11

#define MAX_P2P_MSG                 12
#define P2P_FUNC_SIZE               (MAX_P2P_STATE * MAX_P2P_MSG)


#define ACT_IDLE                      0
#define MAX_ACT_STATE                 1

#define ACT_MACHINE_BASE              0
//Those PEER_xx_CATE number is based on real Categary value in IEEE spec. Shouldn't modify it by your self.
//Category 
#define MT2_PEER_SPECTRUM_CATE              0
#define MT2_PEER_QOS_CATE              1
#define MT2_PEER_DLS_CATE             2
#define MT2_PEER_BA_CATE             3
#define MT2_PEER_PUBLIC_CATE             4
#define MT2_PEER_RM_CATE             5
#define MT2_PEER_HT_CATE             7  //  7.4.7
#define MAX_PEER_CATE_MSG                   21
#define MT2_MLME_ADD_BA_CATE             8
#define MT2_MLME_ORI_DELBA_CATE             9
#define MT2_MLME_REC_DELBA_CATE             10
#define MT2_MLME_QOS_CATE              11
#define MT2_MLME_DLS_CATE             12
#define MT2_ACT_INVALID             13
#ifdef WFD_NEW_PUBLIC_ACTION
#define MT2_MLME_ACTION_CATE             14
#define MT2_PEER_VHT_CATE 21
#define MT2_MLME_VHT_OPERATING_MODE_NOTIFICATION 22
#define MAX_ACT_MSG                   23
#else
#define MT2_PEER_VHT_CATE 21
#define MT2_MLME_VHT_OPERATING_MODE_NOTIFICATION 22
#define MAX_ACT_MSG                   23
#endif

#define MT2_ACT_VENDOR             0x7f

//Category field
#define CATEGORY_SPECTRUM   0
#define CATEGORY_QOS    1
#define CATEGORY_DLS    2
#define CATEGORY_BA 3
#define CATEGORY_PUBLIC 4
#define CATEGORY_RM 5
#define CATEGORY_HT 7   // Also called Public in 802.11y
#define CATEGORY_TDLS   12  // 11z
#define CATEGORY_VHT 21 //11ac
#define CATEGORY_VENDOR     0x7f    // 11z

#define CATEGORY_SAQUERY 8      // 11w
#define CATEGORY_PROTECT_DUAL_PUBLIC_ACTION 9   //11w
//#define CATEGORY_VENDOR_SPECIFIC_PROTECT 126    //11w

//Spectrum  Action field value 802.11h 7.4.1
#define SPEC_MRQ    0   // Request
#define SPEC_MRP    1   //Report
#define SPEC_TPCRQ  2
#define SPEC_TPCRP  3
#define SPEC_CHANNEL_SWITCH 4

//BA  Action field value
#define ADDBA_REQ   0
#define ADDBA_RESP  1
#define DELBA   2

//Public's  Action field value in Public Category.  Some in 802.11k and some in 11y or 11n
#define COEXIST_2040_ACTION 0   // 11n
#define DSE_ENABLEMENT      1   // 11y
#define DSE_DEENABLEMENT        2   // 11y
#define DSE_REGISTERED_LOCATION     3   // 11y
#define EXTENTED_CHANNEL_SWITCH 4   // 11y
#define DSE_MEASUREMENT_REQ     5   // 11y
#define DSE_MEASUREMENT_REPORT      6   // 11y
#define MEASUREMENT_PILOT_ACTION        7  // 11k
#define DSE_POWER_CONSTRAINT            8   // 11y
#define P2P_ACTION              9   // 11y
#define GAS_INITIAL_REQ         10  // 11U
#define GAS_INITIAL_RSP         11  // 11U
#define GAS_COMEBACK_REQ        12  // 11U
#define GAS_COMEBACK_RSP        13  // 11U
#define VENDOR_PUBLIC_ACT       0x7f    // P2P

//HT  Action field value
#define Notify_BW_ACTION    0
#define SMPS_ACTION     1
#define PSMP_ACTION   2
#define SETPCO_ACTION   3
#define MIMO_CHA_MEASURE_ACTION     4
#define NON_COPMRESSED_BEAMFORMING      5
#define COMPRESSED_BEAMFORMING      6
#define ANTENNA_SELECT          7
#define HT_INFO_EXCHANGE            8

#define ACT_FUNC_SIZE                 (MAX_ACT_STATE * MAX_ACT_MSG)
//
// STA's AUTHENTICATION state machine: states, evvents, total function #
//
#define AUTH_REQ_IDLE                   0
#define AUTH_WAIT_SEQ2                  1
#define AUTH_WAIT_SEQ4                  2
#define MAX_AUTH_STATE                  3

#define AUTH_MACHINE_BASE               0
#define MT2_MLME_AUTH_REQ               0
#define MT2_PEER_AUTH_EVEN              1
#define MT2_AUTH_TIMEOUT                2
#define MAX_AUTH_MSG                    3

#define AUTH_FUNC_SIZE                  (MAX_AUTH_STATE * MAX_AUTH_MSG)

//
// STA's AUTH_RSP state machine: states, events, total function #
//
#define AUTH_RSP_IDLE                   0
#define AUTH_RSP_WAIT_CHAL              1
#define MAX_AUTH_RSP_STATE              2

#define AUTH_RSP_MACHINE_BASE           0
#define MT2_AUTH_CHALLENGE_TIMEOUT      0
#define MT2_PEER_AUTH_ODD               1
#define MT2_PEER_DEAUTH                 2
#define MAX_AUTH_RSP_MSG                3

#define AUTH_RSP_FUNC_SIZE              (MAX_AUTH_RSP_STATE * MAX_AUTH_RSP_MSG)

//
// STA's SYNC state machine: states, events, total function #
//
#define SYNC_IDLE                       0  // merge NO_BSS,IBSS_IDLE,IBSS_ACTIVE and BSS in to 1 state
#define JOIN_WAIT_BEACON                1
#define SCAN_LISTEN                     2
#define SCAN_PENDING                    3
#define MAX_SYNC_STATE                  4

#define SYNC_MACHINE_BASE               0
#define MT2_MLME_SCAN_REQ               0
#define MT2_MLME_JOIN_REQ               1
#define MT2_MLME_START_REQ              2
#define MT2_PEER_BEACON                 3
#define MT2_PEER_PROBE_RSP              4
#define MT2_PEER_ATIM                   5
#define MT2_SCAN_TIMEOUT                6
#define MT2_BEACON_TIMEOUT              7
#define MT2_ATIM_TIMEOUT                8
#define MT2_PEER_PROBE_REQ              9
#define MT2_RESTORE_CHANNEL_TIMEOUT     10
#define MAX_SYNC_MSG                    11

#define SYNC_FUNC_SIZE                  (MAX_SYNC_STATE * MAX_SYNC_MSG)

//
// STA's WPA-PSK State machine: states, events, total function #
// 
#define WPA_PSK_IDLE                    0
#define MAX_WPA_PSK_STATE               1

#define WPA_MACHINE_BASE                0
#define MT2_EAPPacket                   0
#define MT2_EAPOLStart                  1
#define MT2_EAPOLLogoff                 2
#define MT2_EAPOLKey                    3
#define MT2_EAPOLASFAlert               4
#define MAX_WPA_PSK_MSG                 5

#define WPA_PSK_FUNC_SIZE               (MAX_WPA_PSK_STATE * MAX_WPA_PSK_MSG)


// WSC State machine
#define WSC_IDLE                        0
#define MAX_WSC_STATE                   2
#define WSC_FUNC_SIZE                   10  // 2 state 3 events

// 
// AP's CONTROL/CONNECT state machine: states, events, total function #
// 
#define AP_CNTL_FUNC_SIZE               1

//
// AP's ASSOC state machine: states, events, total function #
//
#define AP_ASSOC_IDLE                   0
#define AP_MAX_ASSOC_STATE              1

#define AP_ASSOC_MACHINE_BASE           0
#define APMT2_MLME_DISASSOC_REQ         0
#define APMT2_PEER_DISASSOC_REQ         1
#define APMT2_PEER_ASSOC_REQ            2
#define APMT2_PEER_REASSOC_REQ          3
#define APMT2_CLS3ERR                   4
#define AP_MAX_ASSOC_MSG                5

#define AP_ASSOC_FUNC_SIZE              (AP_MAX_ASSOC_STATE * AP_MAX_ASSOC_MSG)

//
// AP's AUTHENTICATION state machine: states, events, total function #
//
#define AP_AUTH_REQ_IDLE                0
#define AP_MAX_AUTH_STATE               1

#define AP_AUTH_MACHINE_BASE            0
#define APMT2_MLME_DEAUTH_REQ           0
#define APMT2_CLS2ERR                   1
#define AP_MAX_AUTH_MSG                 2

#define AP_AUTH_FUNC_SIZE               (AP_MAX_AUTH_STATE * AP_MAX_AUTH_MSG)

//
// AP's AUTH-RSP state machine: states, events, total function #
//
#define AP_AUTH_RSP_IDLE                0
#define AP_MAX_AUTH_RSP_STATE           1

#define AP_AUTH_RSP_MACHINE_BASE        0
#define APMT2_AUTH_CHALLENGE_TIMEOUT    0
#define APMT2_PEER_AUTH_ODD             1
#define APMT2_PEER_DEAUTH               2
#define AP_MAX_AUTH_RSP_MSG             3

#define AP_AUTH_RSP_FUNC_SIZE           (AP_MAX_AUTH_RSP_STATE * AP_MAX_AUTH_RSP_MSG)

//
// AP's SYNC state machine: states, events, total function #
//
#define AP_SYNC_IDLE                    0 
#define AP_MAX_SYNC_STATE               1

#define AP_SYNC_MACHINE_BASE            0
#define APMT2_PEER_PROBE_REQ            0
#define APMT2_PEER_BEACON               1
#define APMT2_PEER_PROBE_RSP            2
#define AP_MAX_SYNC_MSG                 3


#define AP_SYNC_FUNC_SIZE               (AP_MAX_SYNC_STATE * AP_MAX_SYNC_MSG)

//
// AP's WPA state machine: states, events, total function #
//
#define AP_WPA_PTK                      0
#define AP_MAX_WPA_PTK_STATE            1

#define AP_WPA_MACHINE_BASE             0
#define APMT2_EAPPacket                 0
#define APMT2_EAPOLStart                1
#define APMT2_EAPOLLogoff               2
#define APMT2_EAPOLKey                  3
#define APMT2_EAPOLASFAlert             4
#define AP_MAX_WPA_MSG                  5

#define AP_WPA_FUNC_SIZE                (AP_MAX_WPA_PTK_STATE * AP_MAX_WPA_MSG)

//
//AP's WAPI state machine: states, events, total function #
//
#define AP_WAPI_PTK                             0
#define AP_MAX_WAPI_STATE                   1

#define AP_WAPI_MACHINE_BASE                0
#define AP_WAPI_MT2_UNICAST_REQ         0
#define AP_WAPI_MT2_MBCAST_REQ          1
#define AP_WAPI_MT2_MULTICAST_REQ      2
#define AP_WAPI_MT2_WAIPacket               3
#define AP_MAX_WAPI_MSG                         4

#define AP_WAPI_FUNC_SIZE                (AP_MAX_WAPI_STATE * AP_MAX_WAPI_MSG)

//
//STA's WAPI state machine: states, events, total function #
//
#define STA_WAPI_WPTK                           0
#define STA_MAX_WAPI_STATE                  1

#define STA_WAPI_MACHINE_BASE                   0
#define STA_WAPI_MT2_WAIPacket              1
#define STA_MAX_WAPI_MSG                        2

#define STA_WAPI_FUNC_SIZE                (STA_MAX_WAPI_STATE * STA_MAX_WAPI_MSG)


// =============================================================================

// value domain of 802.11 header FC.Tyte, which is b3..b2 of the 1st-byte of MAC header
#define BTYPE_MGMT                  0
#define BTYPE_CNTL                  1
#define BTYPE_DATA                  2

// value domain of 802.11 MGMT frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header
#define SUBTYPE_ASSOC_REQ           0
#define SUBTYPE_ASSOC_RSP           1
#define SUBTYPE_REASSOC_REQ         2
#define SUBTYPE_REASSOC_RSP         3
#define SUBTYPE_PROBE_REQ           4
#define SUBTYPE_PROBE_RSP           5
#define SUBTYPE_BEACON              8
#define SUBTYPE_ATIM                9
#define SUBTYPE_DISASSOC            10
#define SUBTYPE_AUTH                11
#define SUBTYPE_DEAUTH              12
#define SUBTYPE_ACTION              13
#define SUBTYPE_ACTION_NO_ACK              14

// value domain of 802.11 CNTL frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header
#define SUBTYPE_WRAPPER         7
#define SUBTYPE_BLOCK_ACK_REQ       8
#define SUBTYPE_BLOCK_ACK           9
#define SUBTYPE_PS_POLL             10
#define SUBTYPE_RTS                 11
#define SUBTYPE_CTS                 12
#define SUBTYPE_ACK                 13
#define SUBTYPE_CFEND               14
#define SUBTYPE_CFEND_CFACK         15

// value domain of 802.11 DATA frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header
#define SUBTYPE_DATA                0
#define SUBTYPE_DATA_CFACK          1
#define SUBTYPE_DATA_CFPOLL         2
#define SUBTYPE_DATA_CFACK_CFPOLL   3
#define SUBTYPE_NULL_FUNC           4
#define SUBTYPE_CFACK               5
#define SUBTYPE_CFPOLL              6
#define SUBTYPE_CFACK_CFPOLL        7
#define SUBTYPE_QDATA               8
#define SUBTYPE_QDATA_CFACK         9
#define SUBTYPE_QDATA_CFPOLL        10
#define SUBTYPE_QDATA_CFACK_CFPOLL  11
#define SUBTYPE_QOS_NULL            12
#define SUBTYPE_QOS_CFACK           13
#define SUBTYPE_QOS_CFPOLL          14
#define SUBTYPE_QOS_CFACK_CFPOLL    15

// ACK policy of QOS Control field bit 6:5
#define NORMAL_ACK                  0x00  // b6:5 = 00
#define NO_ACK                      0x20  // b6:5 = 01
#define NO_EXPLICIT_ACK             0x40  // b6:5 = 10
#define BLOCK_ACK                   0x60  // b6:5 = 11

//
// rtmp_data.c use these definition
//
#define LENGTH_802_11               24
#define LENGTH_802_11_QOS               26  // 802.11 header + Qos 2 bytes
#define LENGTH_802_11_QOS_PAD               28  // 802.11 header + Qos 2 bytes + pad 2 byt
#define LENGTH_802_11_AND_H         30
#define LENGTH_802_11_CRC_H         34
#define LENGTH_802_11_CRC           28
#define LENGTH_802_11_WITH_ADDR4    30
#define LENGTH_802_3                14
#define LENGTH_802_3_TYPE           2
#define LENGTH_802_1_H              8
#define LENGTH_EAPOL_H              4
#define LENGTH_CRC                  4
#define MAX_SEQ_NUMBER              0x0fff
#define LENGTH_WEP_IV               4
#define LENGTH_WEP_EIV              4
#define LENGTH_WEP_ICV              4
#define LENGTH_TKIP_MIC             8
#define LENGTH_CKIP_MIC             4
#define LENGTH_CKIP_SEQ             4

// STA_CSR4.field.TxResult
#define TX_RESULT_SUCCESS           0
#define TX_RESULT_ZERO_LENGTH       1
#define TX_RESULT_UNDER_RUN         2
#define TX_RESULT_OHY_ERROR         4
#define TX_RESULT_RETRY_FAIL        6

// MCS for CCK.  BW.SGI.STBC are reserved
#define MCS_LONGP_RATE_1                      0  // long preamble CCK 1Mbps
#define MCS_LONGP_RATE_2                      1 // long preamble CCK 1Mbps
#define MCS_LONGP_RATE_5_5                    2
#define MCS_LONGP_RATE_11                     3
#define MCS_SHORTP_RATE_1                      4     // long preamble CCK 1Mbps. short is forbidden in 1Mbps
#define MCS_SHORTP_RATE_2                      5    // short preamble CCK 2Mbps
#define MCS_SHORTP_RATE_5_5                    6
#define MCS_SHORTP_RATE_11                     7
// To send duplicate legacy OFDM. set BW=BW_40.  SGI.STBC are reserved
#define MCS_RATE_6                      0   // legacy OFDM
#define MCS_RATE_9                      1   // OFDM
#define MCS_RATE_12                     2   // OFDM
#define MCS_RATE_18                     3   // OFDM
#define MCS_RATE_24                     4  // OFDM
#define MCS_RATE_36                     5   // OFDM
#define MCS_RATE_48                     6  // OFDM
#define MCS_RATE_54                     7 // OFDM   
// HT
#define MCS_0       0   // 1S
#define MCS_1       1
#define MCS_2       2
#define MCS_3       3
#define MCS_4       4
#define MCS_5       5
#define MCS_6       6
#define MCS_7       7
#define MCS_8       8   // 2S
#define MCS_9       9
#define MCS_10      10
#define MCS_11      11
#define MCS_12      12
#define MCS_13      13
#define MCS_14      14
#define MCS_15      15
#define MCS_16      16  // 3*3
#define MCS_17      17
#define MCS_18      18
#define MCS_19      19
#define MCS_20      20
#define MCS_21      21
#define MCS_22      22
#define MCS_23      23
#define MCS_32      32
#define MCS_AUTO        33
#define MCS_AUTO_SGI_GF     77

// Multichannel control constant 
#define CHANNEL_OFF 0xff
//#define EXTCHA_OFF        0xff

// OID_HTPHYMODE
// Extension channel offset
#define EXTCHA_NONE 0    
#define EXTCHA_ABOVE    0x1
#define EXTCHA_BELOW    0x3

// Primary channel location, indicate this for BBP
//    For the BW 40
#define PRI_CH_LOWER_20     0x01    // Primary channel is locate lower then central channel 10MHz
#define PRI_CH_HIGHER_20    0x03    // Primary channel is locate higher then central channel 10MHz
//    For the BW 80
#define PRI_CH_LOWER_30     0x00    // Primary channel is locate lower then central channel 30MHz
#define PRI_CH_LOWER_10     0x01    // Primary channel is locate lower then central channel 10MHz
#define PRI_CH_HIGHER_10    0x02    // Primary channel is locate higher then central channel 10MHz
#define PRI_CH_HIGHER_30    0x03    // Primary channel is locate higher then central channel 30MHz

// SHORTGI
#define GI_400      1   // only support in HT/VHT mode
#define GI_800  0   
#define GI_BOTH     2
// STBC
#define STBC_NONE   0
#define STBC_USE    1   // limited use in rt2860b phy
#define RXSTBC_ONE  1   // rx support of one spatial stream
#define RXSTBC_TWO  2   // rx support of 1 and 2 spatial stream
#define RXSTBC_THR  3   // rx support of 1~3 spatial stream
// MCS FEEDBACK
#define MCSFBK_NONE 0  // not support mcs feedback /
#define MCSFBK_RSV  1   // reserved
#define MCSFBK_UNSOLICIT    2   // only support unsolict mcs feedback
#define MCSFBK_MRQ  3   // response to both MRQ and unsolict mcs feedback

// MIMO power safe 
#define MMPS_STATIC 0
#define MMPS_DYNAMIC        1
#define   MMPS_RSV      2
#define   MMPS_ENABLE       3

// A-MSDU size
#define AMSDU_0 0
#define AMSDU_1     1

// MCS use 7 bits 
#define TXRATEMIMO      0x80
#define TXRATEMCS       0x7F
#define TXRATEOFDM      0x7F
#define RATE_1                      0
#define RATE_2                      1
#define RATE_5_5                    2
#define RATE_11                     3
#define RATE_6                      4   // OFDM
#define RATE_9                      5   // OFDM
#define RATE_12                     6   // OFDM
#define RATE_18                     7   // OFDM
#define RATE_24                     8   // OFDM
#define RATE_36                     9   // OFDM
#define RATE_48                     10  // OFDM
#define RATE_54                     11  // OFDM
#define RATE_FIRST_OFDM_RATE        RATE_6
#define RATE_LAST_OFDM_RATE         RATE_54
#define RATE_AUTO_SWITCH            255 // for StaCfg.FixedTxRate only
#define HTRATE_0                      12
#define RATE_FIRST_MM_RATE        HTRATE_0
#define RATE_FIRST_HT_RATE        HTRATE_0
#define RATE_LAST_HT_RATE        HTRATE_0

#define CCK_RATE                    1
#define OFDM_RATE                   2
#define CCKOFDM_RATE                3

#define AC0_DEF_TXOP                0
#define AC1_DEF_TXOP                0
#define AC2_DEF_TXOP                94
#define AC3_DEF_TXOP                47

// pTxWI->txop
#define IFS_HTTXOP                 0
#define IFS_PIFS                1 // Not use
#define IFS_SIFS                2
//#define IFS_BACKOFF              3 // Not use

// pTxD->RetryMode
#define LONG_RETRY                  1
#define SHORT_RETRY                 0

//3 ***********************************************************************************************************************
//3 How to add new country region code?
//3 1.) Add new REGION_XXX_BG_BAND or REGION_XXX_A_BAND definitions.
//3 2.) Update UI_MAX_REGION_CODE_IN_BG_BAND or UI_MAX_REGION_CODE_IN_A_BAND.
//3 ***********************************************************************************************************************

// Country Region definition
#define REGION_MINIMUM_BG_BAND            0
#define REGION_0_BG_BAND                  0       // 1-11
#define REGION_1_BG_BAND                  1       // 1-13
#define REGION_2_BG_BAND                  2       // 10-11
#define REGION_3_BG_BAND                  3       // 10-13
#define REGION_4_BG_BAND                  4       // 14
#define REGION_5_BG_BAND                  5       // 1-14
#define REGION_6_BG_BAND                  6       // 3-9
#define REGION_7_BG_BAND                  7       // 5-13
#define REGION_30_BG_BAND           30    // Manual channel
#define REGION_31_BG_BAND                 31      // 1-14, 1-11:active scan, 12-14:passive scan
#define REGION_32_BG_BAND                 32      // 1-13, 1-11:active scan, 12-13:passive scan
#define REGION_33_BG_BAND                 33      // 1-14, all active scan, 802.11g/n: ch14 disallowed
#define REGION_MAXIMUM_BG_BAND            33

#define REGION_MINIMUM_A_BAND             0
#define REGION_0_A_BAND                   0       // 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165
#define REGION_1_A_BAND                   1       // 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140
#define REGION_2_A_BAND                   2       // 36, 40, 44, 48, 52, 56, 60, 64
#define REGION_3_A_BAND                   3       // 52, 56, 60, 64, 149, 153, 157, 161
#define REGION_4_A_BAND                   4       // 149, 153, 157, 161, 165
#define REGION_5_A_BAND                   5       // 149, 153, 157, 161
#define REGION_6_A_BAND                   6       // 36, 40, 44, 48
#define REGION_7_A_BAND                   7       // 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, 
#define REGION_8_A_BAND                   8       // 52, 56, 60, 64
#define REGION_9_A_BAND                   9       // 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165
#define REGION_10_A_BAND                 10       // 36, 40, 44, 48,149, 153, 157, 161, 165
#define REGION_11_A_BAND                 11       // 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 149, 153, 157, 161
//REGION_12_A_BAND mode will also enable bIEEE80211H & CarrierDetect
#define REGION_12_A_BAND                 12       // 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 
#define REGION_13_A_BAND                 13       // 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161
#define REGION_14_A_BAND                 14       // 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 136, 140, 149, 153, 157, 161, 165
#define REGION_15_A_BAND                   15       // 149, 153, 157, 161, 165, 169, 173
#define REGION_16_A_BAND                 16       // 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, 169, 173
#define REGION_17_A_BAND            17      // 36, 40, 44, 48, 52, 56, 60, 64, 132, 136, 140
#define REGION_30_A_BAND              30       // Manual channel
#define REGION_MAXIMUM_A_BAND            30

// Channel Mode
#define ChMode_24G      0
#define ChMode_24G_5G   1
#define ChMode_5G       2

//
// The maximum country region code in BG band that the Ralink UI can display
//
#define UI_MAX_REGION_CODE_IN_BG_BAND       REGION_7_BG_BAND

//
// The maximum country region code in A band that the Ralink UI can display
//
#define UI_MAX_REGION_CODE_IN_A_BAND        REGION_17_A_BAND

// pTxD->CipherAlg
#define CIPHER_NONE                 0
#define CIPHER_WEP64                1
#define CIPHER_WEP128               2
#define CIPHER_TKIP                 3
#define CIPHER_AES                  4
#define CIPHER_WAPI             8
#define CIPHER_TKIP_NO_MIC          9       // MIC appended by driver: not a valid value in hardware key table
#define CIPHER_SMS4                 10
#define CIPHER_WEP152               11
#define CIPHER_BIP                 12

// value domain for pAd->HwCfg.RfIcType
#define RFIC_2820                   1       // 2.4G 2T3R
#define RFIC_2850                   2       // 2.4G/5G 2T3R
#define RFIC_2720                   3       // 2.4G 1T2R
#define RFIC_2750                   4       // 2.4G/5G 1T2R
#define RFIC_3020                   5       // 2.4G 1T1R
#define RFIC_2020                   6       // 2.4G legacy
#define RFIC_3021                   7       // 2.4G 1T2R
#define RFIC_3022                   8       // 2.4G 2T2R
#define RFIC_3052                   9       // 2.4G/5G 2T2R
#define RFIC_3320             11       // 2.4G 1T1R
#define RFIC_3053             13      // 2.4G/5G 3T3R
#define RFIC_7603             14      // 2.4G/5G 3T3R

// LED Status.
#define LED_LINK_DOWN               0
#define LED_LINK_UP                 1
#define LED_RADIO_OFF               2
#define LED_RADIO_ON                3
#define LED_HALT                    4
#define LED_WPS                     5
#define LED_ON_SITE_SURVEY          6
#define LED_POWER_UP                7

#define BT_LED_OFF                  0
#define BT_LED_ON                   1

// MCU command.
#define MCU_SET_WPS_LED_MODE                0x55 // Set WPS LED mode (based on WPS specification V1.0).
#define MCU_SET_ANT_DIVERSITY               0x73
#define MCU_SET_BT_ACTIVE_POLLING           0x90 // If enable, firmware polling GPIO0 to enable/disable MAC TX


// WPS LED status (for LED mode = 7 only)
#define LED_WPS_IN_PROCESS              8  // The protocol is searching for a partner, connecting, or exchanging network parameters.
#define LED_WPS_ERROR                   9  // Some error occurred which was not related to security, such as failed to find any partner or protocol prematurely aborted.
#define LED_WPS_SESSION_OVERLAP_DETECTED        10 // The Protocol detected overlapping operation (more than one Registrar in PBC mode): could be a security risk.
#define LED_WPS_SUCCESS                 11 // The protocol is finished: no uncorrectable errors occured. Normally after guard time period.
#define LED_WPS_TURN_LED_OFF                12 // Turn the WPS LEDs off.
#define LED_WPS_TURN_ON_BLUE_LED                    13 // Turn on the WPS blue LED.
#define LED_NORMAL_CONNECTION_WITHOUT_SECURITY  14 // Successful connection with an AP using OPEN-NONE.
#define LED_NORMAL_CONNECTION_WITH_SECURITY     15 // Successful connection with an AP using an encryption algorithm.

// WPS LED mode (for LED mode = 7 and 8 only)
#define LINK_STATUS_WPS_IN_PROCESS          0x00 // The protocol is searching for a partner, connecting, or exchanging network parameters.
#define LINK_STATUS_WPS_SUCCESS_WITH_SECURITY       0x01 // The protocol is finished (with security): no uncorrectable errors occured. Normally after guard time period.
#define LINK_STATUS_WPS_ERROR               0x02 // Some error occurred which was not related to security, such as failed to find any partner or protocol prematurely aborted.
#define LINK_STATUS_WPS_SESSION_OVERLAP_DETECTED    0x03 // The Protocol detected overlapping operation (more than one Registrar in PBC mode): could be a security risk.
#define LINK_STATUS_WPS_TURN_LED_OFF            0x04 // Turn the WPS LEDs off.
#define LINK_STATUS_WPS_SUCCESS_WITHOUT_SECURITY    0X05 // The protocol is finished (without security): no uncorrectable errors occured. Normally after guard time period.
#define LINK_STATUS_NORMAL_CONNECTION_WITHOUT_SECURITY  0x06 // Successful connection with an AP using OPEN-NONE.
#define LINK_STATUS_NORMAL_CONNECTION_WITH_SECURITY     0x0E // Successful connection with an AP using an encryption algorithm.
#define LINK_STATUS_WPS_BLUE_LED                            0x01 // WPS blue LED.

//WPS LED mode (for LED mode = 10 only)
#define LED_WPS_MODE10_TURN_ON          16  // For Dlink WPS LED, turn the WPS LED on
#define LED_WPS_MODE10_FLASH                17  // For Dlink WPS LED, let the WPS LED flash
#define LED_WPS_MODE10_TURN_OFF         18  // For Dlink WPS LED, turn the WPS LED off

#define LED_MAX_STATE                       19

#define LINK_STATUS_WPS_MODE10_TURN_ON      0x00    //Use only on Dlink WPS LED (mode 10), turn the WPS LED on.
#define LINK_STATUS_WPS_MODE10_FLASH            0x01    //Use only on Dlink WPS LED (mode 10), let the WPS LED flash, three times persecond.
#define LINK_STATUS_WPS_MODE10_TURN_OFF     0x02    //Use only on Dlink WPS LED (mode 10), turn the WPS LED off.

//WPS LED mode (for LED mode = 15 only)
#define LINK_STATUS_WPS_SUCCESS     0x0E 


//
// LED mode
// EEPROM location: 0x3Bh
//
#define LED_MODE 0x7F

//
// WPS LED mode.
//
#define WPS_LED_MODE_6 0x06
#define WPS_LED_MODE_7 0x07
#define WPS_LED_MODE_8 0x08
#define WPS_LED_MODE_9 0x09
#define WPS_LED_MODE_10 0x0a
#define WPS_LED_MODE_12 0x0c
#define WPS_LED_MODE_14 0x0E
#define WPS_LED_MODE_15 0x0F


// value domain of pAd->HwCfg.LedCntl.LedMode and E2PROM
#define LED_MODE_DEFAULT            0
#define LED_MODE_TWO_LED            1
#define LED_MODE_SIGNAL_STREGTH     0x40

// RC4 init value, used fro WEP & TKIP
#define PPPINITFCS32                0xffffffff   /* Initial FCS value */

// value domain of pAd->StaCfg.PortSecured. 802.1X controlled port definition
#define WPA_802_1X_PORT_SECURED     1
#define WPA_802_1X_PORT_NOT_SECURED 2

#define PAIRWISE_KEY                1
#define GROUP_KEY                   2

#define PCI_CFG_ADDR_PORT           0xcf8
#define PCI_CFG_DATA_PORT           0xcfc

// vendor ID
#define RICOH                       0x1180
#define O2MICRO                     0x1217
#define TI                          0x104c
#define RALINK                      0x1814
#define TOSHIBA                     0x1179
#define ENE                         0x1524
#define UNKNOWN                     0xffff

#define CARD_BRIDGE_CLASS           0x0607          // CardBus bridge class & subclass

#define MAX_PCI_DEVICE              32      // support up to 32 devices per bus
#define MAX_PCI_BUS                 32      // support 10 buses
#define MAX_FUNC_NUM                4

//definition of DRS
#define MAX_STEP_OF_TX_RATE_SWITCH  32

// pre-allocated free NDIS PACKET/BUFFER poll for internal usage
#define MAX_NUM_OF_FREE_NDIS_PACKET 128

//Block ACK
#define Max_TX_REORDERBUF   100 //64
#define Max_RX_REORDERBUF   100 //64
#define DEFAULT_TX_TIMEOUT   30
#define DEFAULT_RX_TIMEOUT   30
#define Max_BARECI_SESSION   8

// definition of Recipient or Originator
#define I_RECIPIENT                  TRUE
#define I_ORIGINATOR                   FALSE

// definition of pAd->OpMode
#define OPMODE_STA                  0
#define OPMODE_AP                   1
#define OPMODE_APCLIENT                   2

//#define OPMODE_L3_BRG               2       // as AP and STA at the same time
#define OPMODE_STAP2P               3

// Carrier detection
#define CARRIER_HIT_THRES           3       // Hit threshold, continous hit count
#define CARRIER_FALSECCA_THRES      150     // FalseCCA threshold
#define CARRIER_ACTION_STOP         0       // stop Carrier/Radar detection
#define CARRIER_ACTION_NO_CTS       1       // Start Carrier/Radar detection without CTS protection
#define CARRIER_ACTION_ONE_CTS      2       // Start Carrier/Radar detection with one CTS protection
#define CARRIER_ACTION_TWO_CTS      3       // Start Carrier/Radar detection with two CTS protection

#ifdef BIG_ENDIAN
#define DIR_READ                    0
#define DIR_WRITE                   1
#define TYPE_TXD                    0
#define TYPE_RXD                    1
#endif

// ========================= AP rtmp_def.h ===========================
// Length definitions
#define AP_MAX_LEN_OF_RSNIE         90
#define AP_MIN_LEN_OF_RSNIE         8

// value domain for pAd->TrackInfo.EventTab.Log[].Event
#define EVENT_RESET_ACCESS_POINT    0 // Log = "hh:mm:ss   Restart Access Point"
#define EVENT_ASSOCIATED            1 // Log = "hh:mm:ss   STA 00:01:02:03:04:05 associated"
#define EVENT_DISASSOCIATED         2 // Log = "hh:mm:ss   STA 00:01:02:03:04:05 left this BSS"
#define EVENT_AGED_OUT              3 // Log = "hh:mm:ss   STA 00:01:02:03:04:05 was aged-out and removed from this BSS"
#define EVENT_COUNTER_M             4
#define EVENT_INVALID_PSK           5
#define EVENT_MAX_EVENT_TYPE        6
// ==== end of AP rtmp_def.h ============

// definition RSSI Number
#define RSSI_NO_1                   0
#define RSSI_NO_2                   1

#define USB_DEVICE_MAX_CONFIG_DESCRIPTOR_SIZE   1024

////////////////////////////////////////////////////////////////////////////
// Frame Sizes
////////////////////////////////////////////////////////////////////////////

#define MAC_ADDRESS_LENGTH              6

#define HEADER_SIZE                     14
#define MAXIMUM_PACKET_SIZE             1500

#define USB_DEVICE_MAX_CONFIG_DESCRIPTOR_SIZE   1024

#define MAX_QUEUE_SIZE                  100

#define USB_TX_HEADER_SIZE              8//WLength+TxRate+PaddingBytes+Reserved
#define USB_RX_HEADER_SIZE              12

#define WIRELESS_HEADER_OVERHEAD        18
#define MAX_TX_PADDING_BYTES            50
#define MAX_RX_PADDING_BYTES            66
#define CRC32_BYTES                     4

#define MAX_WIRELESS_SIZE               WIRELESS_HEADER_OVERHEAD + HEADER_SIZE + MAXIMUM_PACKET_SIZE + CRC32_BYTES

typedef UCHAR ADDRESS[MAC_ADDRESS_LENGTH];


// definition RSSI Number
#define RSSI_0                  0
#define RSSI_1                  1
#define RSSI_2                  2

// definition of radar detection
#define RD_NORMAL_MODE              0   // Not found radar signal
#define RD_SWITCHING_MODE           1   // Found radar signal, and doing channel switch
#define RD_SILENCE_MODE             2   // After channel switch, need to be silence a while to ensure radar not found
#define  SLEEPCID   0x11
#define  WAKECID    0x22
#define  QUERYPOWERCID  0x33
#define  LEDMODECID 0x44
#define  SIGSTRENCID    0x55
#define  OWNERMCU   0x1
#define  OWNERCPU   0x0


// definition for DLS
#define MAX_NUM_OF_INIT_DLS_ENTRY   1
#define MAX_NUM_OF_DLS_ENTRY        4

// DLS Action frame definition
#define ACTION_DLS_REQUEST          0
#define ACTION_DLS_RESPONSE         1
#define ACTION_DLS_TEARDOWN         2


// 
// STA's DLS State machine: states, events, total function #
//
#define DLS_IDLE                        0
#define MAX_DLS_STATE                   1

#define DLS_MACHINE_BASE                0
#define MT2_MLME_DLS_REQ                0
#define MT2_PEER_DLS_REQ                1
#define MT2_PEER_DLS_RSP                2
#define MT2_MLME_DLS_TEAR_DOWN          3
#define MT2_PEER_DLS_TEAR_DOWN          4
#define MAX_DLS_MSG                     5

#define DLS_FUNC_SIZE                   (MAX_DLS_STATE * MAX_DLS_MSG)


/** The Max Frame size does not include the FCS */
#define MP_802_11_MAX_FRAME_SIZE                (DOT11_MAX_PDU_SIZE - 4)
#define MP_802_11_MIN_FRAME_SIZE                DOT11_MIN_PDU_SIZE
#define MP_802_11_SHORT_HEADER_SIZE             DOT11_DATA_SHORT_HEADER_SIZE
#define MP_802_11_LONG_HEADER_SIZE              DOT11_DATA_LONG_HEADER_SIZE
#define MP_802_11_MAX_FRAGMENTS_PER_MSDU        DOT11_MAX_NUM_OF_FRAGMENTS
#define MP_802_11_MAXIMUM_LOOKAHEAD                                             \
                (DOT11_MIN_PDU_SIZE - (                                     \
                            sizeof(DOT11_DATA_SHORT_HEADER) + 12))

/** The max link speed supported by this hardware (in bps) */
#define HW_LINK_SPEED                       600000000 //600Mbps, // Temporary increase size for VHT throughput tuning(ToDo: Review appropriate size)
#define HW_REPORT_MAX_SPEED                 600000000 //600Mbps, // Temporary increase size for VHT throughput tuning(ToDo: Review appropriate size)

/** Maximum number of on-NIC multicast address entries */
#define HW_MAX_MCAST_LIST_SIZE    32

/** The backfill size this miniport driver needs. This is the maximum size of encryption IV */
#define HW_REQUIRED_BACKFILL_SIZE         8

/** Maximum number of MAC addresses we support in the excluded list */
#define STA_EXCLUDED_MAC_ADDRESS_MAX_COUNT      4
/** Max number of desired BSSIDs we can handle */
#define STA_DESIRED_BSSID_MAX_COUNT             8
/** Max number of desired PHYs we can handle */
#define STA_DESIRED_PHY_MAX_COUNT               5
/** Max number of PMKID we can handle */
#define STA_PMKID_MAX_COUNT                     3
/** Max number of enabled multicast cipher algorithms */
#define STA_MULTICAST_CIPHER_MAX_COUNT          6

#define STA_MAX_SCAN_SSID_LIST_COUNT            4 // minimum required for DOT11_EXTSTA_CAPABILITY

#define STA_BSS_ENTRY_EXPIRE_TIME               100000000       // 10 seconds/

#define KEY_TABLE_SIZE                       16
#define KEY_MAPPING_KEY_TABLE_SIZE           (KEY_TABLE_SIZE - DOT11_MAX_NUM_DEFAULT_KEY)
#define PER_STA_KEY_TABLE_SIZE               KEY_MAPPING_KEY_TABLE_SIZE

#define STA_AGED_OUT_TIME                       100000000       // 10 seconds used by NdisGetCurrentSystemTime
#define TEN_SECOND                              STA_AGED_OUT_TIME
#define SIX_SECOND                          60000000

//define for ExtAPAttributes
#define AP_SCAN_SSID_LIST_MAX_SIZE              4
#define AP_DESIRED_SSID_LIST_MAX_SIZE           1
#define AP_PRIVACY_EXEMPTION_LIST_MAX_SIZE      32
#define AP_DEFAULT_ALLOWED_ASSOCIATION_COUNT    64
#define AP_STRICTLY_ORDERED_SERVICE_CLASS_IMPLEMENTED FALSE

#define  NUM_SUPPORTED_VWIFI_COMBINATIONS   3

#define AP_DESIRED_PHY_MAX_COUNT   1

//
// win8 or win7
//
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  

#define RT_VWIFI_COMBINATION_REVISION           DOT11_VWIFI_COMBINATION_REVISION_3
#define RT_SIZEOF_VWIFI_COMBINATION_REVISION    DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_3
typedef PDOT11_VWIFI_COMBINATION_V3 PRT_VWIFI_COMBINATION;

#define MULTI_BSSID_MODE MULTI_BSSID_8

#else

#define RT_VWIFI_COMBINATION_REVISION           DOT11_VWIFI_COMBINATION_REVISION_2
#define RT_SIZEOF_VWIFI_COMBINATION_REVISION    DOT11_SIZEOF_VWIFI_COMBINATION_REVISION_2
typedef PDOT11_VWIFI_COMBINATION_V2 PRT_VWIFI_COMBINATION;

#define MULTI_BSSID_MODE MULTI_BSSID_2

#endif

#if 0
#define Ralink802_11AuthModeOpen                Ndis802_11AuthModeOpen
#define Ralink802_11AuthModeShared              Ndis802_11AuthModeShared
#define Ralink802_11AuthModeAutoSwitch          Ndis802_11AuthModeAutoSwitch
#define Ralink802_11AuthModeWPANone             Ndis802_11AuthModeWPANone
#define Ralink802_11AuthModeWPA                 Ndis802_11AuthModeWPA
#define Ralink802_11AuthModeWPAPSK              Ndis802_11AuthModeWPAPSK
#define Ralink802_11AuthModeWPA2                Ndis802_11AuthModeWPA2
#define Ralink802_11AuthModeWPA2PSK             Ndis802_11AuthModeWPA2PSK
#define Ralink802_11AuthModeWPA1WPA2            Ndis802_11AuthModeWPA1WPA2
#define Ralink802_11AuthModeWPA1PSKWPA2PSK      Ndis802_11AuthModeWPA1PSKWPA2PSK


#define Ralink802_11WEPDisabled                 Ndis802_11WEPDisabled
#define Ralink802_11EncryptionDisabled          Ndis802_11EncryptionDisabled
#define Ralink802_11WEPEnabled                  Ndis802_11WEPEnabled
//#define Ralink802_11Encryption1Enabled            Ndis802_11Encryption1Enabled
#define IS_WEP_STATUS_ON(_Wep)                  (_Wep == Ndis802_11WEPEnabled)
#define Ralink802_11Encryption2Enabled          Ndis802_11Encryption2Enabled
#define Ralink802_11Encryption3Enabled          Ndis802_11Encryption3Enabled
#define Ralink802_11Encryption4Enabled          Ndis802_11Encryption4Enabled



#else
#define Ralink802_11AuthModeOpen                DOT11_AUTH_ALGO_80211_OPEN
#define Ralink802_11AuthModeShared              DOT11_AUTH_ALGO_80211_SHARED_KEY
#define Ralink802_11AuthModeWPANone             DOT11_AUTH_ALGO_WPA_NONE
#define Ralink802_11AuthModeWPA                 DOT11_AUTH_ALGO_WPA
#define Ralink802_11AuthModeWPAPSK              DOT11_AUTH_ALGO_WPA_PSK
#define Ralink802_11AuthModeWPA2                DOT11_AUTH_ALGO_RSNA
#define Ralink802_11AuthModeWPA2PSK             DOT11_AUTH_ALGO_RSNA_PSK
#define Ralink802_11AuthModeWPA1WPA2            (ULONG)(DOT11_AUTH_ALGO_IHV_START)          //Ndis802_11AuthModeWPA1WPA2
#define Ralink802_11AuthModeWPA1PSKWPA2PSK      (ULONG)(DOT11_AUTH_ALGO_IHV_START + 1)      //Ndis802_11AuthModeWPA1PSKWPA2PSK
#define Ralink802_11AuthModeAutoSwitch          (ULONG)(DOT11_AUTH_ALGO_IHV_START + 4)

#define Ralink802_11WEPDisabled                 DOT11_CIPHER_ALGO_NONE
#define Ralink802_11EncryptionDisabled          DOT11_CIPHER_ALGO_NONE
#define Ralink802_11WEPEnabled                  DOT11_CIPHER_ALGO_WEP
#define Ralink802_11Encryption1Enabled          DOT11_CIPHER_ALGO_WEP
#define IS_WEP_STATUS_ON(_Wep)                  ((_Wep == DOT11_CIPHER_ALGO_WEP40) || (_Wep == DOT11_CIPHER_ALGO_WEP104) || (_Wep == DOT11_CIPHER_ALGO_WEP))
#define Ralink802_11Encryption2Enabled          DOT11_CIPHER_ALGO_TKIP
#define Ralink802_11Encryption3Enabled          DOT11_CIPHER_ALGO_CCMP
#define Ralink802_11Encryption4Enabled          (ULONG)(DOT11_CIPHER_ALGO_IHV_START)            //TKIP/AES mix mode

// [WAPI]
//
//#define Ralink802_11EncryptionWPI_SMS4              (ULONG)(DOT11_CIPHER_ALGO_IHV_START+3)
//#define Ralink802_11AuthModeWAI                 (ULONG)(DOT11_AUTH_ALGO_IHV_START + 2)

//
// Virtual Station
//
#define VPORT_DOT11_CIPHER_ALGO_CCMP            (ULONG)(DOT11_CIPHER_ALGO_IHV_START + 8)
#define VPORT_DOT11_AUTH_ALGO_RSNA_PSK          (ULONG)(DOT11_AUTH_ALGO_IHV_START + 9)


//#define Ralink802_11AuthModeWAI_PSK             (ULONG)(Ralink802_11AuthModeWAI + 0x100)    
//#define Ralink802_11AuthModeWAI_CERT                (ULONG)(Ralink802_11AuthModeWAI + 0x101)
//#define Ralink802_11AuthModeWAI_PSKCERT         (ULONG)(Ralink802_11AuthModeWAI + 0x102)
//#define Ndis802_11AuthModeWAI_PSK                   Ralink802_11AuthModeWAI_PSK
//#define Ndis802_11AuthModeWAI_CERT              Ralink802_11AuthModeWAI_CERT
//#define Ndis802_11EncryptionWPI_SMS4                Ralink802_11EncryptionWPI_SMS4

//#define IS_WEP_STATUS_ON(_Wep)                    ((_Wep == DOT11_CIPHER_ALGO_WEP40) || (_Wep == DOT11_CIPHER_ALGO_WEP104) || (_Wep == DOT11_CIPHER_ALGO_WEP))
#endif

// [PMF] //need to change for feet Win8
//
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
#define DOT11_CIPHER_ALGO_BIP                       (ULONG)(DOT11_CIPHER_ALGO_IHV_START + 4) //add for future implementation
#endif


//AdhocNMode
#define ADHOCNMODE_DISABLE      0

#define WAKE_MCU_CMD                0x31
#define SLEEP_MCU_CMD               0x30
#define RFOFF_MCU_CMD               0x35
#define RFTUNE_MCU_CMD              0x74

// Reposit Bssid List
#define MAX_REPOSIT_BSSID_LIST_SIZE     128*1024
#define DEFAULT_REPOSIT_TIME            20  //20 seconds as default
#define MIN_REPOSIT_TIME                0   //0 seconds as minimum
#define MAX_REPOSIT_TIME                255 // 4.25 minutes as maximum

#define ALLOW_CHANGE_MCAST_RATE 0x10000000  // Bit 28 on: Allow change multicast rate, Bit[0~15] means HTTRANSMIT_SETTING
#define USER_DEFINE_MCAST_RATE  0x10010000  // Bit 16 on: Use user define multicast rate, Bit[0~15] means HTTRANSMIT_SETTING
#define USE_LEGACY_MCAST_RATE       0x10100000  // Bit 20 on: Force to use legacy rate for multicast, Bit[0~15] means HTT
#define USE_AUTO_MCAST_RATE     0x11000000  // Bit 20 on: Force to use legacy rate for multicast, Bit[0~15] means HTT

// 
// STA's TDLS State machine: states, events, total function #
//
#define TDLS_IDLE                   0
#define MAX_TDLS_STATE              1

#define TDLS_MACHINE_BASE               0
#define MT2_MLME_TDLS_SETUP_REQ         0
#define MT2_PEER_TDLS_MSG               1
#define MT2_MLME_TDLS_TEAR_DOWN         2
#define MAX_TDLS_MSG                    3

#define TDLS_FUNC_SIZE              (MAX_TDLS_STATE * MAX_TDLS_MSG)

// TDLS Action field values
#define ACTION_TDLS_SETUP_REQ               0
#define ACTION_TDLS_SETUP_RSP               1
#define ACTION_TDLS_SETUP_CONF              2
#define ACTION_TDLS_TEARDOWN                3
#define ACTION_TDLS_PEER_TRAFFIC_IND        4
#define ACTION_TDLS_CHANNEL_SWITCH_REQ      5
#define ACTION_TDLS_CHANNEL_SWITCH_RSP      6
#define ACTION_TDLS_PEER_PSM_REQ            7
#define ACTION_TDLS_PEER_PSM_RSP            8
#define ACTION_TDLS_AP_PHY_DATA_RATE_RSP    9

#define MAX_NUM_OF_TDLS_ENTRY       MAX_NUM_OF_DLS_ENTRY
#define TDLS_ENTRY_AGEOUT_TIME      30      // unit: sec
#define TDLS_MAX_SEARCH_TABLE_SZIE  64

#define TDLS_SEARCH_ENTRY_AGEOUT    5  /* seconds */
#define TDLS_SEARCH_ENTRY_LIFTTIME  600  /* seconds */
#define TDLS_SEARCH_POOL_SIZE       256
#define TDLS_SEARCH_HASH_TAB_SIZE   256  /* the legth of hash table must be power of 2. */
#define TDLS_SEARCH_ENTRY_RETRY_TIMES   5

// 802.11z definitions
#define PROTO_NAME_TDLS             2
#define TDLS_AKM_SUITE              7   // TPK Handshake


#define DIRECT_LINK_MODE_DLS        0
#define DIRECT_LINK_MODE_TDLS       1

//
// Below is for 802.11r
//

#define LEN_PMK_NAME            16
#define LEN_NONCE               32
#define LEN_PMK                 32

// define the fast-roaming sensitivity degrees
#define LOWEST_SENSITIVITY          1
#define MEDIUM_LOW_SENSITIVITY      2
#define MEDIUM_SENSITIVITY          3
#define MEDIUM_HIGH_SENSITIVITY     4
#define HIGHEST_SENSITIVITY         5

// define the corresponding dBm value of each fast-roaming sensitivity degree
#define LOWEST_SENSITIVITY_DBM          80
#define MEDIUM_LOW_SENSITIVITY_DBM      75
#define MEDIUM_SENSITIVITY_DBM          70
#define MEDIUM_HIGH_SENSITIVITY_DBM     65
#define HIGHEST_SENSITIVITY_DBM         60

// Channel define
#define CHANNEL_14              14

//
// Definition for WMI Event
//
#define MAX_MESSAGE_EVENT_LENGTH          1024
#define RAWLAN_MESSAGE_EVENT_INDICATION  ((NDIS_STATUS)0x60010002L)
#define RAWLAN_TLV_DATA_SIZE        (64 - 8)    // The element of TLV Data size are defined in MOF file


// Define Message Event
#define EXTREG_MESSAGE_EVENT_ID         0x07    // for WPS external registrar set event (M2,M4,M6,M8).

//Disable Asic Sync
#define DisableAPBT 0
#define DisableAP   1
#define DisableBT   2

#if UAPSD_AP_SUPPORT
#define WMM_NUM_OF_AC                   4  /* AC_BE, AC_BK,AC_VI, AC_VO  */
#define NUM_OF_UAPSD_CLI                2 /* max clients we support*/
#endif

// For Access CR
#define CR_READ 0
#define CR_WRITE 1
#define VENDOR_REQUEST_READ 0x63
#define VENDOR_REQUEST_WRITE    0x66
#define VENDOR_REQUEST_READ_EFUSE   0xb
#define VENDOR_REQUEST_WRITE_EFUSE  0xa

#define CR_BASE_HIF             0x50000000
#define CR_BASE_UDMA            0x50029000

#if 0
#define UDMA_WLCFG_0            (CR_BASE_UDMA + 0x0018) 
#define STOP_DROP_EPOUT         (CR_BASE_UDMA + 0x0080) 

#define QUEUE_PRIORITY1     (CR_BASE_HIF + 0x580)
#define QUEUE_PRIORITY2     (CR_BASE_HIF + 0x584)

#define SCH_REG1                (CR_BASE_HIF + 0x588) 
#define SCH_REG2                (CR_BASE_HIF + 0x58c) 
#define SCH_REG3                (CR_BASE_HIF + 0x590) 
#define SCH_REG4                (CR_BASE_HIF + 0x594) 
#define GROUP_THD0              (CR_BASE_HIF + 0x598)
#define GROUP_THD1              (CR_BASE_HIF + 0x59c)
#define GROUP_THD2              (CR_BASE_HIF + 0x5a0)
#define GROUP_THD3              (CR_BASE_HIF + 0x5a4)
#define GROUP_THD4              (CR_BASE_HIF + 0x5a8)
#define GROUP_THD5              (CR_BASE_HIF + 0x5ac)
#define BMAP0                   (CR_BASE_HIF + 0x5b0)
#define BMAP1                   (CR_BASE_HIF + 0x5b4)
#define BMAP2                   (CR_BASE_HIF + 0x5b8)
#define HIGH_PRIORITY1          (CR_BASE_HIF + 0x5bc)
#define HIGH_PRIORITY2          (CR_BASE_HIF + 0x5c0)
#define PRIORITY_MASK           (CR_BASE_HIF + 0x5c4)
#define RSV_MAX_THD         (CR_BASE_HIF + 0x5c8)
#endif

// WF_MAC
#define CR_BASE_WF_CFG          0x60000000
#define CR_BASE_WF_TRB          0x60100000
#define CR_BASE_WF_AGG      0x60110000
#define CR_BASE_WF_ARB          0x60120000
#define CR_BASE_WF_TMAC     0x60130000
#define CR_BASE_WF_RMAC     0x60140000
#define CR_BASE_WF_SEC          0x60150000
#define CR_BASE_WF_DMA      0x60160000
#define CR_BASE_WF_CFGOFF       0x60170000
#define CR_BASE_WF_PF           0x60180000
#define CR_BASE_WF_WTBLOFF  0x60190000
#define CR_BASE_WF_ETBF     0x601A0000

#define CR_BASE_WF_LPON     0x60300000
#define CR_BASE_WF_INT          0x60310000
#define CR_BASE_WF_WTBLON       0x60320000
#define CR_BASE_WF_MIB          0x60330000
#define CR_BASE_WF_AON      0x60400000

#define CR_BASE_TOP             0x80020000

#if 0
// WF_AGG
#define AGG_AALCR               (CR_BASE_WF_AGG + 0x040)
#define AGG_AALCR1              (CR_BASE_WF_AGG + 0x044)
#define AGG_AWSCR               (CR_BASE_WF_AGG + 0x048)
#define AGG_AWSCR1              (CR_BASE_WF_AGG + 0x04c)
#define AGG_ASRCR               (CR_BASE_WF_AGG + 0x060)
#define AGG_ACR                 (CR_BASE_WF_AGG + 0x070)
#define AGG_MRCR                (CR_BASE_WF_AGG + 0x0f4)
#define AGG_BWCR                (CR_BASE_WF_AGG + 0x0ec)
#define AGG_DSCR1               (CR_BASE_WF_AGG + 0x0b4)

// WF_ARB
#define ARB_RQCR_RX_START       0x01
#define ARB_RQCR_RXV_START      0x10
#define ARB_RQCR_RXV_R_EN       0x80
#define ARB_RQCR_RXV_T_EN       0x100
#define MT_ARB_SCR_TXDIS        (1 << 8)
#define MT_ARB_SCR_RXDIS        (1 << 9)

#define ARB_RQCR                (CR_BASE_WF_ARB + 0x070)
#define ARB_SCR                 (CR_BASE_WF_ARB + 0x080)
#define ARB_TQCR0               (CR_BASE_WF_ARB + 0x100)
#define ARB_TQCR1               (CR_BASE_WF_ARB + 0x104)
#define ARB_TQCR4               (CR_BASE_WF_ARB + 0x110)
#define ARB_TQCR5               (CR_BASE_WF_ARB + 0x114)
#define ARB_BCNQCR0             (CR_BASE_WF_ARB + 0x118)

#define ARB_GTQR0               (CR_BASE_WF_ARB + 0x010)
#define ARB_GTQR1               (CR_BASE_WF_ARB + 0x014)
#define ARB_GTQR2               (CR_BASE_WF_ARB + 0x018)

// WF_MIB
#define MIB_MSCR                (CR_BASE_WF_MIB + 0x00)
#define MIB_MPBSCR              (CR_BASE_WF_MIB + 0x04)

// WF_TMAC
#define TMAC_TCR                (CR_BASE_WF_TMAC + 0x00)
#define TMAC_TRCR               (CR_BASE_WF_TMAC + 0x9C)

// WF_RMAC
#define RMAC_RFCR               (CR_BASE_WF_RMAC + 0x00)
#define RMAC_CB0R0              (CR_BASE_WF_RMAC + 0x04)
#define RMAC_CB0R1              (CR_BASE_WF_RMAC + 0x08)
#define RMAC_CB1R0              (CR_BASE_WF_RMAC + 0x0C)
#define RMAC_CB1R1              (CR_BASE_WF_RMAC + 0x10)
#define RMAC_CB2R0              (CR_BASE_WF_RMAC + 0x14)
#define RMAC_CB2R1              (CR_BASE_WF_RMAC + 0x18)
#define RMAC_OMA0R0         (CR_BASE_WF_RMAC + 0x024)
#define RMAC_OMA0R1         (CR_BASE_WF_RMAC + 0x028)
#define RMAC_OMA1R0             (CR_BASE_WF_RMAC + 0x02C)
#define RMAC_OMA1R1             (CR_BASE_WF_RMAC + 0x030)
#define RMAC_OMA2R0             (CR_BASE_WF_RMAC + 0x034)
#define RMAC_OMA2R1             (CR_BASE_WF_RMAC + 0x038)
#define RMAC_RMCR               (CR_BASE_WF_RMAC + 0x080)
#define RMAC_MISC               (CR_BASE_WF_RMAC + 0x094)

#define RMAC_RMCR_SMPS_MODE     (0x3 << 20)
#define RMAC_RMCR_SMPS_BY_RTS       (1 << 25)
#define RMAC_RMCR_RX_STREAM_0       1<<22
#define RMAC_RMCR_RX_STREAM_1       1<<23
#define RMAC_RMCR_RX_STREAM_2       1<<24


// WF_DMA
#define DMA_DCR1                (CR_BASE_WF_DMA + 0x004)
#define DMA_RCFR0               (CR_BASE_WF_DMA + 0x070)
#define DMA_VCFR0               (CR_BASE_WF_DMA + 0x07c)
#define DMA_TCFR0               (CR_BASE_WF_DMA + 0x080)
#define DMA_TCFR1               (CR_BASE_WF_DMA + 0x084)

// WF_WTBLON
#define WTBL_WTBL1DW0           (CR_BASE_WF_WTBLON + 0x00)
#define WTBL_WTBL1DW1           (CR_BASE_WF_WTBLON + 0x04)
#define WTBL_WTBL1DW2           (CR_BASE_WF_WTBLON + 0x08)
#define WTBL_WTBL1DW3           (CR_BASE_WF_WTBLON + 0x0C)
#define WTBL_WTBL1DW4           (CR_BASE_WF_WTBLON + 0x10)

#define WTBL_BTCRn              (CR_BASE_WF_WTBLON + 0x2000)
#define WTBL_BTBCRn             (CR_BASE_WF_WTBLON + 0x2030)
#define WTBL_BRCRn              (CR_BASE_WF_WTBLON + 0x2100)
#define WTBL_BRBCRn             (CR_BASE_WF_WTBLON + 0x2130)
#define WTBL_MBTCRn         (CR_BASE_WF_WTBLON + 0x2010)
#define WTBL_MBRCRn         (CR_BASE_WF_WTBLON + 0x2110)
#define WTBL_MBTBCRn            (CR_BASE_WF_WTBLON + 0x2040)
#define WTBL_MBRBCRn            (CR_BASE_WF_WTBLON + 0x2140)

// WF_LPON
#define LPON_T0TPCR             (CR_BASE_WF_LPON + 0x34)
#define LPON_T1TPCR             (CR_BASE_WF_LPON + 0x38)
#define LPON_TTSR               (CR_BASE_WF_LPON + 0x3c)
#define LPON_TFRSR              (CR_BASE_WF_LPON + 0x40)
#define LPON_TSELR              (CR_BASE_WF_LPON + 0x44)
#define LPON_SPCR               (CR_BASE_WF_LPON + 0x48)
#define LPON_BCNTR              (CR_BASE_WF_LPON + 0x4c)
#define LPON_TCLCR              (CR_BASE_WF_LPON + 0x50)
#define LPON_MPTCR0             (CR_BASE_WF_LPON + 0x5c)
#define LPON_MPTCR1             (CR_BASE_WF_LPON + 0x60)
#define LPON_FRCR               (CR_BASE_WF_LPON + 0x7c)
#define LPON_WLANCKCR           (CR_BASE_WF_LPON + 0x88)
#define BEACONPERIODn_MASK (0xffff)
#define BEACONPERIODn(p) (((p) & 0xffff))
#define DTIMPERIODn_MASK (0xff << 16)
#define DTIMPERIODn(p) (((p) & 0xff) << 16)
#define TBTTWAKEPERIODn_MASK (0xf << 24)
#define TBTTWAKEPERIODn(p) (((p) & 0xf) << 24)
#define DTIMWAKEPERIODn_MASK (0x7 << 28)
#define DTIMWAKEPERIODn(p) (((p) & 0x7) << 28)
#define TBTTn_CAL_EN (1 << 31)


#define TOP_MISC2               (CR_BASE_TOP + 0x1134) 


#define WF_MIB_BASE     0x2C000
#define MIB_MSDR0   (WF_MIB_BASE + 0x08)
#define MIB_MSDR6   (WF_MIB_BASE + 0x20)
#define MIB_MSDR7   (WF_MIB_BASE + 0x24)
#define MIB_MSDR8   (WF_MIB_BASE + 0x28)
#define MIB_MSDR9   (WF_MIB_BASE + 0x2c)
#define MIB_MSDR10  (WF_MIB_BASE + 0x30)
#define MIB_MSDR11  (WF_MIB_BASE + 0x34)
#define MIB_MSDR12  (WF_MIB_BASE + 0x38)
#define MIB_MSDR14  (WF_MIB_BASE + 0x40)
#define MIB_MSDR15  (WF_MIB_BASE + 0x44)
#define MIB_MSDR16  (WF_MIB_BASE + 0x48)
#define MIB_MSDR17  (WF_MIB_BASE + 0x4c)
#define MIB_MSDR18  (WF_MIB_BASE + 0x50)
#define MIB_MSDR22  (WF_MIB_BASE + 0x60)
#define MIB_MSDR23  (WF_MIB_BASE + 0x64)
#define MIB_MDR0        (WF_MIB_BASE + 0xa0)
#define MIB_MDR1        (WF_MIB_BASE + 0xa4)
#define MIB_MDR2        (WF_MIB_BASE + 0xa8)
#define MIB_MDR3        (WF_MIB_BASE + 0xac)
#define MIB_MB0SDR0     (WF_MIB_BASE + 0x100)
#define MIB_MB1SDR0     (WF_MIB_BASE + 0x110)
#define MIB_MB2SDR0     (WF_MIB_BASE + 0x120)
#define MIB_MB3SDR0     (WF_MIB_BASE + 0x130)
#define MIB_MB0SDR1     (WF_MIB_BASE + 0x104)
#define MIB_MB1SDR1     (WF_MIB_BASE + 0x114)
#define MIB_MB2SDR1     (WF_MIB_BASE + 0x124)
#define MIB_MB3SDR1     (WF_MIB_BASE + 0x134)
#define MIB_MB0SDR2     (WF_MIB_BASE + 0x108)
#define MIB_MB1SDR2     (WF_MIB_BASE + 0x118)
#define MIB_MB2SDR2     (WF_MIB_BASE + 0x128)
#define MIB_MB3SDR2     (WF_MIB_BASE + 0x138)
#define MIB_MSDR4   (WF_MIB_BASE + 0x18)
#define MIB_MSDR5   (WF_MIB_BASE + 0x1c)
#endif

#define NORMAL_CMD_MASK             0x10000000
#define P2P_CMD_MASK                    0x20000000
#define MULTICHANNEL_CMD_MASK  0x40000000

#define INTERNAL_CMD_BUFFER_LEN 256
#define INTERNAL_CMD_NUM            32


//Normal cmd
#define MT_CMD_AP_RESET_REQUEST                                                 (NORMAL_CMD_MASK|0x00000001)
#define MT_CMD_STA_RESET_REQUEST                                               (NORMAL_CMD_MASK|0x00000002)
#define MT_CMD_STA_UPDTAE_DEFAULT_ID                                        (NORMAL_CMD_MASK|0x00000003)
#define MT_CMD_RESTART_AP_IS_REQUIRED                                      (NORMAL_CMD_MASK|0x00000004)
#define MT_CMD_ENABLE_PROMISCUOUS_MODE                                  (NORMAL_CMD_MASK|0x00000005)
#define MT_CMD_DISABLE_PROMISCUOUS_MODE                                 (NORMAL_CMD_MASK|0x00000006)
#define MT_CMD_ADD_STA_MAPPING_KEY                                           (NORMAL_CMD_MASK|0x00000007)
#define MT_CMD_ADD_STA_DEFAULT_KEY                                            (NORMAL_CMD_MASK|0x00000008)
#define MT_CMD_ADD_AP_DEFAULT_KEY                                             (NORMAL_CMD_MASK|0x00000009)
#define MT_CMD_ADD_AP_MAPPING_KEY                                             (NORMAL_CMD_MASK|0x0000000A)
#define MT_CMD_RTMP_SHUTDOWN                                                    (NORMAL_CMD_MASK|0x0000000B)
#define MT_CMD_FORCE_SLEEP                                                            (NORMAL_CMD_MASK|0x0000000C)
#define MT_CMD_DISCONNECT_REQUEST                                              (NORMAL_CMD_MASK|0x0000000D)
#define MT_CMD_SET_ASIC_WCID                                                        (NORMAL_CMD_MASK|0x0000000E)
#define MT_CMD_SET_ASIC_WCID_CIPHER                                          (NORMAL_CMD_MASK|0x0000000F)
#define MT_CMD_SET_DLS_KEY_TABLE                                                (NORMAL_CMD_MASK|0x00000010)
#define MT_CMD_SET_DLS_RX_WCID_TABLE                                        (NORMAL_CMD_MASK|0x00000011)
#define MT_CMD_TXBF_NDPANNOUNCE_GEN                                        (NORMAL_CMD_MASK|0x00000012)
#define MT_CMD_SET_TBTT_TIMER                                                      (NORMAL_CMD_MASK|0x00000013)
#define MT_CMD_UPDATE_TIM                                                             (NORMAL_CMD_MASK|0x00000014)
#define MT_CMD_RESET_BBP                                                                (NORMAL_CMD_MASK|0x00000015)
#define MT_CMD_CTRL_PWR_BIT                                                          (NORMAL_CMD_MASK|0x00000016) // Update the power bit in control frame
#define MT_CMD_SEND_BEACON                                                            (NORMAL_CMD_MASK|0x00000017)
#define MT_CMD_PNP_SET_POWER                                                       (NORMAL_CMD_MASK|0x00000018)
#define MT_CMD_RESEND_RX_IRP                                                        (NORMAL_CMD_MASK|0x00000019)
#define MT_CMD_CHECK_GPIO                                                              (NORMAL_CMD_MASK|0x0000001A)
#define MT_CMD_DOT11_RTS_THRESHOLD                                            (NORMAL_CMD_MASK|0x0000001B)
#define MT_CMD_DOT11_POWER_MGMT_REQUEST                                (NORMAL_CMD_MASK|0x0000001C)
#define MT_CMD_PERIODIC_EXECUT                                                     (NORMAL_CMD_MASK|0x0000001D)
#define MT_CMD_PERIODIC_CHECK_RX                                                 (NORMAL_CMD_MASK|0x0000001E)
#define MT_CMD_USB_RESET_BULK_IN                                                 (NORMAL_CMD_MASK|0x0000001F)
#define MT_CMD_QKERIODIC_EXECUT                                                   (NORMAL_CMD_MASK|0x00000020)
#define MT_CMD_MULTI_WRITE_MAC                                                    (NORMAL_CMD_MASK|0x00000021)
#define MT_CMD_USB_RESET_BULK_OUT                                              (NORMAL_CMD_MASK|0x00000022)
#define MT_CMD_ASIC_ADD_SKEY                                                         (NORMAL_CMD_MASK|0x00000023)
#define MT_CMD_USB_VENDOR_RESET                                                  (NORMAL_CMD_MASK|0x00000024)
#define MT_CMD_802_11_SET_PHY_MODE                                             (NORMAL_CMD_MASK|0x00000025)
#define MT_CMD_PERFORM_SOFT_DIVERSITY                                       (NORMAL_CMD_MASK|0x00000026)
#define MT_CMD_MLME_RESTART_STATE_MACHINE                               (NORMAL_CMD_MASK|0x00000027)
#define MT_CMD_LED_WPS_MODE10                                                     (NORMAL_CMD_MASK|0x000000028)


//P2p related
#define MT_CMD_P2P_STOP_GO                                                          (P2P_CMD_MASK|0x00000001)
#define MT_CMD_P2P_STOP_CLIENT                                                    (P2P_CMD_MASK|0x00000002)
#define MT_CMD_P2P_PERIODIC_EXECUT                                            (P2P_CMD_MASK|0x00000003)
#define MT_CMD_P2P_SWNOATIMEOUT                                                (P2P_CMD_MASK|0x00000004)
#define MT_CMD_P2pMs_RADIO_ON                                                      (P2P_CMD_MASK|0x00000005)
#define MT_CMD_P2P_UPDATE_BEACON                                                (P2P_CMD_MASK|0x00000006)
#define MT_CMD_P2pMs_SEND_COMPLETE_IND                                     (P2P_CMD_MASK|0x00000007)
#define MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_RCV                         (P2P_CMD_MASK|0x00000008)
#define MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_EXEC                        (P2P_CMD_MASK|0x00000009)
#define MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_SEND_EXEC              (P2P_CMD_MASK|0x0000000A)
#define MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_GOT_ACK_EXEC        (P2P_CMD_MASK|0x0000000B)
#define MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_TIMEOUT_EXEC        (P2P_CMD_MASK|0x0000000C)
#define MT_CMD_P2P_READ_TXFIFO                                                     (P2P_CMD_MASK|0x0000000D)
#define MT_CMD_P2P_SET_PSM_CFG                                                    (P2P_CMD_MASK|0x0000000E)

//MultiChannel related
#define MT_CMD_MULTI_CHANNEL_SWITCH_EXEC                                 (MULTICHANNEL_CMD_MASK|0x00000001)
#define MT_CMD_MULTI_CHANNEL_P2pMs_START_NEW_SESSION         (MULTICHANNEL_CMD_MASK|0x00000002)
#define MT_CMD_MULTI_CHANNEL_P2pMs_CH_RESTORE                       (MULTICHANNEL_CMD_MASK|0x00000003)
#define MT_CMD_MLME_PERIODIC_EXEC_PASSIVE_LEVEL                      (MULTICHANNEL_CMD_MASK|0x00000004)
#define MT_CMD_MULTI_CHANNEL_SCAN_START_EXEC                          (MULTICHANNEL_CMD_MASK|0x00000005)
#define MT_CMD_MULTI_CHANNEL_SCAN_DONE_EXEC                           (MULTICHANNEL_CMD_MASK|0x00000006)

typedef enum 
{
    PENDING_SCAN_REQUEST = 0,
    PENDING_PNP_SET_POWER = 1,
    PENDING_CREATE_DELETE_MAC = 2,
    PENDING_NIC_POWER_STATE = 3,
    PENDING_ENUM_BSS_LIST = 4,
    PENDING_DEFAULT = 5,
}PENDING_OID_TYPE, * PPENDING_OID_TYPE;

typedef struct _MIB_DATA
{
    UINT32 mibmscr;
    UINT32 mibmpbscr;
    UINT32 idx;
    UINT32 ampdu_cnt[5];
    UINT32 msdr6;
    UINT32 msdr7;
    UINT32 msdr8;
    UINT32 msdr9; 
    UINT32 msdr10;
    UINT32 msdr16;
    UINT32 msdr17;
    UINT32 msdr18;
    UINT32 mibmsdr0;
    UINT32 mibmdr0;
    UINT32 mibmdr1;
    UINT32 mibmsdr4;
    UINT32 mibmsdr5;
    UINT32 mibmsdr22;
    UINT32 mibmsdr23;
    UINT32 mbxsdr[4][3];
    UINT32 mbtcr[16];
    UINT32 mbtbcr[16];
    UINT32 mbrcr[16]; 
    UINT32 mbrbcr[16];
    UINT32 btcr[4];
    UINT32 btbcr[4];
    UINT32 brcr[4];
    UINT32 brbcr[4];
} MIB_DATA, *PMIB_DATA;

/* Rate Adaptation timing */
#define RA_RATE     5                   /* RA every fifth 100msec period */
#define RA_INTERVAL     (RA_RATE*100)   /* RA Interval in msec */

#define DEF_RA_TIME_INTRVAL         500
#define DEF_QUICK_RA_TIME_INTERVAL  100

#define DOT11_N_SUPPORT 1
#define NEW_RATE_ADAPT_SUPPORT 1
#define CONFIG_STA_SUPPORT 1
#define MCS_LUT_SUPPORT 1
#define RTMP_MAC_USB 1
#define RTMP_USB_SUPPORT 1
#define DMA_SCH_SUPPORT 1

#define MT7603 1

#define SHORT_PREAMBLE 0
#define LONG_PREAMBLE 1

#endif  // __RTMP_DEF_H__
