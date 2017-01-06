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
    oid.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
*/
#ifndef _OID_H_
#define _OID_H_

typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;

// New for MeetingHouse Api support
#define MTK_OID_N6_SET_MH_802_1X_SUPPORTED                     (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFFEDC100)


#define MTK_OID_N5_QUERY_VENDOR_COUNTERS                  0x0D73011B

#define MTK_OID_N5_QUERY_USB_VENDOR_RESET                     0x0D730101
#define MTK_OID_N5_QUERY_USB_VENDOR_UNPLUG                    0x0D730102
#define MTK_OID_N5_QUERY_USB_VENDOR_SWITCH_FUNCTION           0x0D730103

#define MTK_OID_N5_QUERY_MULTI_WRITE_MAC                      0x0D730107
#define MTK_OID_N5_QUERY_MULTI_READ_MAC                       0x0D730108
#define MTK_OID_N5_QUERY_USB_VENDOR_EEPROM_WRITE              0x0D73010A
#define MTK_OID_N5_QUERY_USB_VENDOR_EEPROM_READ               0x0D73010B

#define MTK_OID_N5_QUERY_USB_VENDOR_ENTER_TESTMODE            0x0D73010C
#define MTK_OID_N5_QUERY_USB_VENDOR_EXIT_TESTMODE             0x0D73010D
#define MTK_OID_N5_QUERY_USB_DEVICE_DESC                  0x0D730110
#define MTK_OID_N5_QUERY_VENDOR_WRITE_BBP                     0x0D730119
#define MTK_OID_N5_QUERY_VENDOR_READ_BBP                      0x0D730118
#define MTK_OID_N5_QUERY_VENDOR_WRITE_RF                      0x0D73011A

#define MTK_OID_N5_QUERY_VENDOR_FLIP_IQ                       0x0D73011D


#define MTK_OID_N5_SET_PER_RATE_TX_RATE_SWITCHING_STRUC 0x0D730123
#define MTK_OID_N5_QUERY_TEST_MODE_BBP_TUNING_MODE        0x0D730125
#define MTK_OID_N5_SET_TEST_MODE_BBP_TUNING_MODE        (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_TEST_MODE_BBP_TUNING_MODE)
#define MTK_OID_N5_QUERY_NOR_FLASH_ERASE_BLOCK                0x0D730126
#define MTK_OID_N5_QUERY_NOR_FLASH_WRITE                      0x0D730127
#define MTK_OID_N5_QUERY_NOR_FLASH_READ                       0x0D730128
#define MTK_OID_N5_QUERY_NOR_FLASH_GET                        0x0D730129
//#define MTK_OID_N5_SET_GENERAL_TX_RATE_SWITCHING_STRUC    0x0D730124
//#define MTK_OID_N5_QUERY_TX_RATE_SWITCHING_COUNTERS     0x0D730125
//used by driver internally
#define MTK_OID_N5_QUERY_USB_RESET_BULK_OUT                   0x0D730210
#define MTK_OID_N5_QUERY_USB_RESET_BULK_IN                    0x0D730211
#define MTK_OID_N5_SET_PSM_BIT_SAVE                     0x0D730212
//#define MTK_OID_N6_SET_RADIO                          0x0D730214
#define MTK_OID_N5_QUERY_UPDATE_TX_RATE                       0x0D730216
#define MTK_OID_N5_QUERY_ADD_KEY_WEP                      0x0D730218
#define MTK_OID_N5_QUERY_RESET_FROM_ERROR                     0x0D73021A
#define MTK_OID_N5_QUERY_LINK_DOWN                            0x0D73021B
#define MTK_OID_N5_QUERY_RESET_FROM_NDIS                      0x0D73021C
#define MTK_OID_N5_QUERY_PERIODIC_EXECUT                      0x0D73021D
#define MTK_OID_N5_QUERY_TEST_PERIODIC_EXECUT                 0x0D73021E
#define MTK_OID_N5_QUERY_ASICLED_EXECUT                       0x0D73021F
#define MTK_OID_N5_QUERY_CHECK_GPIO                           0x0D730215
#define MTK_OID_N5_QUERY_REMOVE_ALLKEYS                       0x0D730220
#define RT_PERFORM_SOFT_DIVERSITY                   0x0D730221
#define MTK_OID_N5_QUERY_FORCE_WAKE_UP                        0x0D730222
#define MTK_OID_N5_SET_PSM_BIT_ACTIVE                   0x0D730223
#define MTK_OID_N5_QUERY_QKERIODIC_EXECUT                     0x0D73023D
#define MTK_OID_N5_SET_CLIENT_MAC_ENTRY                 0x0D73023E
#define MTK_OID_N5_QUERY_ASIC_ADD_SKEY                        0x0D73023F
#define MTK_OID_N5_QUERY_MLME_RESTART_STATE_MACHINE           0x0D730240
#define MTK_OID_N5_QUERY_PERIODIC_CHECK_RX                    0x0D730243
#define MTK_OID_N5_QUERY_RESEND_RX_IRP                        0x0D73024F
// Ralink defined OIDs for Vista
#if(COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))  
#define MTK_TOGGLE_OID_N6                           0xFF000000
#else
#define MTK_TOGGLE_OID_N6                           0x00000000
#endif

// Ralink defined OIDs
#define MTK_TOGGLE_OID_SET                    0x00800000

#ifndef MTK_OID_N5_QUERY_TEST
#define MTK_OID_N5_QUERY_TEST                       0x0D010120
#endif
#define MTK_OID_N5_SET_BSSID                   (MTK_TOGGLE_OID_SET | OID_802_11_BSSID)
#define MTK_OID_N5_SET_SSID                    (MTK_TOGGLE_OID_SET | OID_802_11_SSID)
#define MTK_OID_N5_SET_INFRASTRUCTURE_MODE     (MTK_TOGGLE_OID_SET | OID_802_11_INFRASTRUCTURE_MODE)
#define MTK_OID_N5_SET_ADD_WEP                 (MTK_TOGGLE_OID_SET | OID_802_11_ADD_WEP)
#define MTK_OID_N5_SET_ADD_KEY                 (MTK_TOGGLE_OID_SET | OID_802_11_ADD_KEY)
#define MTK_OID_N5_SET_REMOVE_WEP              (MTK_TOGGLE_OID_SET | OID_802_11_REMOVE_WEP)
#define MTK_OID_N5_SET_REMOVE_KEY              (MTK_TOGGLE_OID_SET | OID_802_11_REMOVE_KEY)
#define MTK_OID_N5_SET_DISASSOCIATE            (MTK_TOGGLE_OID_SET | OID_802_11_DISASSOCIATE)
#define MTK_OID_N5_SET_AUTHENTICATION_MODE     (MTK_TOGGLE_OID_SET | OID_802_11_AUTHENTICATION_MODE)
#define MTK_OID_N5_SET_PRIVACY_FILTER          (MTK_TOGGLE_OID_SET | OID_802_11_PRIVACY_FILTER)
#define MTK_OID_N5_SET_BSSID_LIST_SCAN         (MTK_TOGGLE_OID_SET | OID_802_11_BSSID_LIST_SCAN)
#define MTK_OID_N5_SET_WEP_STATUS              (MTK_TOGGLE_OID_SET | OID_802_11_WEP_STATUS)
#define MTK_OID_N5_SET_RELOAD_DEFAULTS         (MTK_TOGGLE_OID_SET | OID_802_11_RELOAD_DEFAULTS)
#define MTK_OID_N5_SET_NETWORK_TYPE_IN_USE     (MTK_TOGGLE_OID_SET | OID_802_11_NETWORK_TYPE_IN_USE)
#define MTK_OID_N5_SET_TX_POWER_LEVEL          (MTK_TOGGLE_OID_SET | OID_802_11_TX_POWER_LEVEL)
#define MTK_OID_N5_SET_RSSI_TRIGGER            (MTK_TOGGLE_OID_SET | OID_802_11_RSSI_TRIGGER)
#define MTK_OID_N5_SET_FRAGMENTATION_THRESHOLD (MTK_TOGGLE_OID_SET | OID_802_11_FRAGMENTATION_THRESHOLD)
#define MTK_OID_N5_SET_RTS_THRESHOLD           (MTK_TOGGLE_OID_SET | OID_802_11_RTS_THRESHOLD)
#define MTK_OID_N5_SET_RX_ANTENNA_SELECTED     (MTK_TOGGLE_OID_SET | OID_802_11_RX_ANTENNA_SELECTED)
#define MTK_OID_N5_SET_TX_ANTENNA_SELECTED     (MTK_TOGGLE_OID_SET | OID_802_11_TX_ANTENNA_SELECTED)
#define MTK_OID_N5_SET_SUPPORTED_RATES         (MTK_TOGGLE_OID_SET | OID_802_11_SUPPORTED_RATES)
#define MTK_OID_N5_SET_DESIRED_RATES           (MTK_TOGGLE_OID_SET | OID_802_11_DESIRED_RATES)
#define MTK_OID_N5_SET_CONFIGURATION           (MTK_TOGGLE_OID_SET | OID_802_11_CONFIGURATION)
#define MTK_OID_N5_SET_POWER_MODE              (MTK_TOGGLE_OID_SET | OID_802_11_POWER_MODE)
//#define MTK_OID_N5_SET_STATISTICS              (MTK_TOGGLE_OID_SET | OID_802_11_STATISTICS)

#define MTK_OID_N6_QUERY_PREAMBLE          (MTK_TOGGLE_OID_N6 | 0x0D710101)
#define MTK_OID_N6_SET_PREAMBLE            (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_PREAMBLE)
#define MTK_OID_N5_QUERY_LINK_STATUS       0x0D710102
#define MTK_OID_N5_SET_RESET_COUNTERS          (MTK_TOGGLE_OID_SET | 0x0D710103)
#define MTK_OID_N5_QUERY_AC_CAM            0x0D710104
#define MTK_OID_N5_SET_AC_CAM              (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_AC_CAM)
#define MTK_OID_N5_QUERY_HARDWARE_REGISTER 0x0D710105
#define MTK_OID_N5_SET_HARDWARE_REGISTER   (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_HARDWARE_REGISTER)
#define MTK_OID_N5_QUERY_MTKCONFIG          0x0D710106
#define MTK_OID_N5_SET_MTKCONFIG            (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_MTKCONFIG)
#define MTK_OID_N5_QUERY_COUNTRY_REGION     0x0D710107
#define MTK_OID_N5_SET_COUNTRY_REGION       (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_COUNTRY_REGION)
#define MTK_OID_N5_QUERY_RADIO             0x0D710108
#define MTK_OID_N5_SET_RADIO               (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_RADIO)
//#define MTK_OID_N5_QUERY_RX_AGC_VGC_TUNING 0x0D710109
//#define MTK_OID_N5_SET_RX_AGC_VGC_TUNING   (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_RX_AGC_VGC_TUNING)
#define MTK_OID_N5_QUERY_EVENT_TABLE       0x0D71010A
#define MTK_OID_N5_QUERY_MAC_TABLE         0x0D71010B
#define MTK_OID_N5_QUERY_PHY_MODE          0x0D71010C
#define MTK_OID_N5_SET_PHY_MODE            (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_PHY_MODE)
//#define MTK_OID_N5_QUERY_TX_PACKET_BURST   0x0D71010D
//#define MTK_OID_N5_SET_TX_PACKET_BURST     (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_TX_PACKET_BURST)
//#define MTK_OID_N5_QUERY_TURBO_MODE        0x0D71010E
//#define MTK_OID_N5_SET_TURBO_MODE          (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_TURBO_MODE)
#define MTK_OID_N5_QUERY_AP_CONFIG         0x0D71010F
#define MTK_OID_N5_SET_AP_CONFIG           (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_AP_CONFIG)
#define MTK_OID_N5_QUERY_ACL               0x0D710110
#define MTK_OID_N5_SET_ACL                 (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_ACL)
#define MTK_OID_N6_QUERY_STA_CONFIG        (MTK_TOGGLE_OID_N6 | 0x0D710111)
#define MTK_OID_N6_SET_STA_CONFIG          (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_STA_CONFIG)
#define MTK_OID_N5_QUERY_WDS               0x0D710112
#define MTK_OID_N5_SET_WDS                 (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_WDS)

// Add by James to fit WPA Soft-AP extra data
#define MTK_OID_N5_QUERY_RADIUS_DATA       0x0D710113
#define MTK_OID_N5_SET_RADIUS_DATA         (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_RADIUS_DATA)
//group key update interval
#define MTK_OID_N5_QUERY_WPA_REKEY         0x0D710114
#define MTK_OID_N5_SET_WPA_REKEY           (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_WPA_REKEY)
//wpa counter measure test
#define MTK_OID_N5_QUERY_MIC_ERROR     0x0D710115
#define MTK_OID_N5_SET_MIC_ERROR       (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_MIC_ERROR)

#define MTK_OID_N5_SET_ADD_WPA                 (MTK_TOGGLE_OID_SET | 0x0D710116)
#define MTK_OID_N5_QUERY_CURRENT_CHANNEL_ID 0x0D710117
#define MTK_OID_N5_QUERY_TX_POWER_LEVEL_1  0x0D710118
#define MTK_OID_N5_SET_TX_POWER_LEVEL_1    (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_TX_POWER_LEVEL_1)
#define MTK_OID_N5_QUERY_EEPROM_VERSION    0x0D710119
#define MTK_OID_N5_QUERY_NOISE_LEVEL       0x0D71011A
#define MTK_OID_N5_QUERY_LAST_RX_RATE      0x0D71011B
#define MTK_OID_N5_QUERY_LAST_TX_RATE      0x0D71011c

#define MTK_OID_N5_SET_LEAP_AUTH_INFO          (MTK_TOGGLE_OID_SET | 0x0D71011C)

// Extra inforamtion, such as scanning, deauthentication, wep error
#define MTK_OID_N5_QUERY_EXTRA_INFO              0x0D71011D
#define MTK_OID_N6_SET_CCX20_INFO              (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D71011E)
#define MTK_OID_N6_QUERY_AIRONETIP_INFO    (MTK_TOGGLE_OID_N6 | 0x0D71011F)


#ifdef SMARTANTENNA_SBSA
//
// Smart Antenna Integrate with InterDigital SBSA (Subscriber-Based Smart Antenna)
//
#define MTK_OID_N5_QUERY_CUSTOM_LAST_RX_BEACON_INFO          0x0D710120
#define MTK_OID_N5_QUERY_CUSTOM_SELECT_ANTENNA_INFO    0x0D710121
#define MTK_OID_N5_SET_CUSTOM_SELECT_ANTENNA_INFO      (MTK_TOGGLE_OID_SET | 0x0D710121)
#define MTK_OID_N5_QUERY_CUSTOM_STEERING_ALGORITHM_PARAMS_INFO  0x0D710122
#define MTK_OID_N5_SET_CUSTOM_STEERING_ALGORITHM_PARAMS_INFO (MTK_TOGGLE_OID_SET | 0x0D710122)
#define MTK_OID_N5_QUERY_CUSTOM_STEERING_ALGORITHM_STATE_INFO    0x0D710123
#define MTK_OID_N5_QUERY_CUSTOM_AVERAGE_RSSI_INFO                0x0D710124 
#endif

#define MTK_OID_N5_QUERY_RSSI_1                  0x0D710125
#define MTK_OID_N5_QUERY_OP_MODE             0x0D710126
#define MTK_OID_N5_SET_OP_MODE               (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_OP_MODE)
#define MTK_OID_N5_SET_TX_RATES            (MTK_TOGGLE_OID_SET | 0x0D710127)
#define MTK_OID_N5_QUERY_IEEE80211H          0x0D710128
#define MTK_OID_N5_SET_IEEE80211H            (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_IEEE80211H)
#define MTK_OID_N5_QUERY_FIRMWARE_VERSION    0x0D710129

#define MTK_OID_N6_QUERY_APSD_SETTING      (MTK_TOGGLE_OID_N6 | 0x0D71012A)
#define MTK_OID_N6_SET_APSD_SETTING        (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_APSD_SETTING)
#define MTK_OID_N6_QUERY_APSD_PSM          (MTK_TOGGLE_OID_N6 | 0x0D71012B)
#define MTK_OID_N6_SET_APSD_PSM            (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_APSD_PSM)

#define MTK_OID_N5_QUERY_AGGREGATION         0x0D71012C
#define MTK_OID_N5_SET_AGGREGATION           (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_AGGREGATION)
#define MTK_OID_N6_QUERY_WMM               (MTK_TOGGLE_OID_N6 | 0x0D71012D)
#define MTK_OID_N6_SET_WMM                 (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_WMM)
#define MTK_OID_N5_QUERY_MULTIPLE_IRP        0x0D71012E
#define MTK_OID_N5_SET_MULTIPLE_IRP          (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_MULTIPLE_IRP)

// WAPI
//#define MTK_OID_N6_QUERY_WAPI_SUPPORT     (MTK_TOGGLE_OID_N6 | 0x0D71012F)
//#define MTK_OID_N6_SET_WAPI_SUPPORT         (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_WAPI_SUPPORT)


#define MTK_OID_N6_QUERY_DLS               (MTK_TOGGLE_OID_N6 | 0x0D710131)
#define MTK_OID_N6_SET_DLS                 (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_DLS)
#define MTK_OID_N6_QUERY_DLS_PARAM         (MTK_TOGGLE_OID_N6 | 0x0D710132)
#define MTK_OID_N6_SET_DLS_PARAM           (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_DLS_PARAM)
#define MTK_OID_N6_QUERY_PORT_USAGE          (MTK_TOGGLE_OID_N6 | 0x0D7101DF)

#define MTK_OID_N5_QUERY_RSSI_2                  0x0D710134
#define MTK_OID_N5_QUERY_SNR_0               0x0D710135
#define MTK_OID_N5_QUERY_SNR_1               0x0D710136


//Block ACK
#define MTK_OID_N5_QUERY_IMME_BA_CAP    0x0D710137
#define MTK_OID_N5_SET_IMME_BA_CAP     (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_IMME_BA_CAP)

// MFP
#define MTK_OID_N6_QUERY_MFP_CONTROL         (MTK_TOGGLE_OID_N6 | 0x0D71013F)
#define MTK_OID_N6_SET_MFP_CONTROL           (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_MFP_CONTROL)

//========================>
// P2P 
#define MTK_OID_N6_QUERY_P2P_STATUS         (MTK_TOGGLE_OID_N6 | 0x0D7101D1)
#define MTK_OID_N6_QUERY_WPSE_STATUS        (MTK_TOGGLE_OID_N6 | 0x0D7101D2)
#define MTK_OID_N6_QUERY_WIFI_DIRECT_LINKSTATE  (MTK_TOGGLE_OID_N6 | 0x0D7101D7)
#define MTK_OID_N6_QUERY_P2P_PERSIST     (MTK_TOGGLE_OID_N6 | 0x0D7101DA)
#define MTK_OID_N6_QUERY_P2P_MAC_INFO       (MTK_TOGGLE_OID_N6  | 0x0D710351)
// Set P2P Event Handle after GUI had created it.
#define MTK_OID_N6_SET_P2P_EVENT            (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101D3)
#define MTK_OID_N6_SET_P2P_EVENT_READY      (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101D6)
#define MTK_OID_N6_SET_P2P_CONFIG           (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101D4)
#define MTK_OID_N6_SET_P2P_IP               (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101D5)
#define MTK_OID_N6_SET_P2P_PERST_TAB        (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101D8)
#define MTK_OID_N6_SET_P2P_GO_PASSPHRASE    (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101D9)
#define MTK_OID_N6_SET_P2P_DISCONNECT_PER_CONNECTION    (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF8C004E)
#define MTK_OID_N6_SET_P2P_PERSIST                      (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101DA)
// SIGMA control on p2p
#define MTK_OID_N6_QUERY_SIGMA_P2P_STATUS   (MTK_TOGGLE_OID_N6 | 0x0D710221)
#define MTK_OID_N6_QUERY_SIGMA_WPS_PINCODE  (MTK_TOGGLE_OID_N6 | 0x0D710222)
#define MTK_OID_N6_QUERY_SIGMA_PSK          (MTK_TOGGLE_OID_N6 | 0x0D710223)

//<========================
#define MTK_OID_N5_QUERY_BATABLE    0x0D710138
#define MTK_OID_N5_SET_ADD_IMME_BA         (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_BATABLE)
#define MTK_OID_N5_SET_TEAR_IMME_BA         (MTK_TOGGLE_OID_SET | 0x0D71013a)

#define MTK_OID_N5_QUERY_HT_PHYMODE    0x0D71013b
#define MTK_OID_N5_SET_HT_PHYMODE         (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_HT_PHYMODE)

#define MTK_OID_N5_SET_PSPXLINK_MODE         (MTK_TOGGLE_OID_SET | 0x0D710150)

// for manual channel list 
#define MTK_OID_N6_QUERY_MANUAL_CH_LIST  (MTK_TOGGLE_OID_N6 | 0x0D7101BC)

// reserve 0x0D7101BD ~ 0x0D7101C1
#define MTK_OID_N5_QUERY_STBC_RX_COUNT             0x0D7101BD
#define MTK_OID_N5_QUERY_STBC_TX_COUNT                 0x0D7101BE

/*
    Bit[3:0] is for Rx number of antenna
    Bit[7:4] is for Tx number of antenna
    Bit[31:8] Reserved
*/
#define MTK_OID_N5_QUERY_SS_NUM                  0x0D7101BF
#define MTK_OID_N5_SET_SS_NUM                    (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_SS_NUM)
#define MTK_OID_N5_QUERY_SNR_2                         0x0D7101C0

//
// TxBf OIDs
// Range: 0x0D710200~0x0D71021F
//

#define MTK_OID_N5_SET_TXBF_ETX_BF_TIMEOUT                      (MTK_TOGGLE_OID_SET | 0x0D710206)

#define MTK_OID_N5_SET_TXBF_ETX_BF_EN                           (MTK_TOGGLE_OID_SET | 0x0D71020A)

#define MTK_OID_N5_SET_TXBF_ETX_BF_NON_COMPRESSED_FEEDBACK  (MTK_TOGGLE_OID_SET | 0x0D71020B)

// BT30: For bluetooth HCI command 
#define MTK_OID_N5_QUERY_BT_HCI_GET_CMD            0x0D7101C2
#define MTK_OID_N5_SET_BT_HCI_SEND_CMD       (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_BT_HCI_GET_CMD)
#define MTK_OID_N5_QUERY_BT_HCI_GET_ACL_DATA   0x0D7101C3
#define MTK_OID_N5_SET_BT_HCI_SEND_ACL_DATA  (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_BT_HCI_GET_ACL_DATA)

//Add Paul Chen for Accton
//#define MTK_OID_N6_QUERY_TX_POWER_LEVEL                  (MTK_TOGGLE_OID_N6 | 0xFF020010)
//#define MTK_OID_N6_SET_TX_POWER_LEVEL              (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_TX_POWER_LEVEL)

//
#define MTK_OID_N6_SET_FRAGMENTATION_THRESHOLD    (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | OID_DOT11_FRAGMENTATION_THRESHOLD)
#define MTK_OID_N6_SET_RTS_THRESHOLD              (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | OID_DOT11_RTS_THRESHOLD)
#define MTK_OID_N6_SET_POWER_MGMT_REQUEST         (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | OID_DOT11_POWER_MGMT_REQUEST)

//
#define MTK_OID_N6_SET_CIPHER_KEY_MAPPING_KEY     (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | OID_DOT11_CIPHER_KEY_MAPPING_KEY)
#define MTK_OID_N6_SET_CIPHER_DEFAULT_KEY         (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | OID_DOT11_CIPHER_DEFAULT_KEY)
#define MTK_OID_N6_SET_CIPHER_DEFAULT_KEY_ID      (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | OID_DOT11_CIPHER_DEFAULT_KEY_ID)

#define MTK_OID_N6_SET_CUSTOMER_BT_RADIO_STATUS     (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D710220)

// [WAPI]
//
//*****************************************************************************
//#define MTK_OID_N6_SET_WLAN_WAPIMODE            (MTK_TOGGLE_OID_N6 | 0xFF300003)
//#define MTK_OID_N6_SET_WAI_RESULT_STATUS    (MTK_TOGGLE_OID_N6 | 0xFF300005)
//#define MTK_OID_N6_SET_WPI_KEY              (MTK_TOGGLE_OID_N6 | 0xFF30000A)

//*****************************************************************************

typedef struct _RT_802_11_STATISTICS
{
    ULONG               Length;             // Length of structure
    LARGE_INTEGER   TransmittedFragmentCount;
    LARGE_INTEGER   MulticastTransmittedFrameCount;
    LARGE_INTEGER   FailedCount;
    LARGE_INTEGER   RetryCount;
    LARGE_INTEGER   MultipleRetryCount;
    LARGE_INTEGER   RTSSuccessCount;
    LARGE_INTEGER   RTSFailureCount;
    LARGE_INTEGER   ACKFailureCount;
    LARGE_INTEGER   FrameDuplicateCount;
    LARGE_INTEGER   ReceivedFragmentCount;
    LARGE_INTEGER   MulticastReceivedFrameCount;
    LARGE_INTEGER   FCSErrorCount;
    LARGE_INTEGER   TKIPLocalMICFailures;
    LARGE_INTEGER   TKIPRemoteMICErrors;
    LARGE_INTEGER   TKIPICVErrors;
    LARGE_INTEGER   TKIPCounterMeasuresInvoked;
    LARGE_INTEGER   TKIPReplays;
    LARGE_INTEGER   CCMPFormatErrors;
    LARGE_INTEGER   CCMPReplays;
    LARGE_INTEGER   CCMPDecryptErrors;
    LARGE_INTEGER   FourWayHandshakeFailures;   
    LARGE_INTEGER   RcvSucFrame;

    // Tx aggregation status
    LARGE_INTEGER   TxAggRange1Count;
    LARGE_INTEGER   TxAggRange2Count;
    LARGE_INTEGER   TxAggRange3Count;
    LARGE_INTEGER   TxAggRange4Count;

    /* for PER debug */
    LARGE_INTEGER   AmpduFailCount;
    LARGE_INTEGER   AmpduSuccessCount;

    ULONG       RxNoBuffer;
} RT_802_11_STATISTICS, *PRT_802_11_STATISTICS;

typedef enum _RT_802_11_PREAMBLE {
    Rt802_11PreambleLong,
    Rt802_11PreambleShort,
    Rt802_11PreambleAuto
} RT_802_11_PREAMBLE, *PRT_802_11_PREAMBLE;



typedef enum _RT_802_11_ADHOC_MODE {
    ADHOC_11B,
    ADHOC_11BG_MIXED,
    ADHOC_11G,
    ADHOC_11A,
    ADHOC_11ABG_MIXED,
    ADHOC_11ABGN_MIXED,   
    ADHOC_11N,
    ADHOC_11GN_MIXED,
    ADHOC_11AN_MIXED,
    ADHOC_11BGN_MIXED,
    ADHOC_11AGN_MIXED,
} RT_802_11_ADHOC_MODE;

// put all proprietery for-query objects here to reduce # of Query_OID
typedef struct _RT_802_11_LINK_STATUS {
    ULONG   CurrTxRate;         // in units of 0.5Mbps
    ULONG   ChannelQuality;     // 0..100 %
    ULONG   TxByteCount;        // both ok and fail
    ULONG   RxByteCount;        // both ok and fail
    ULONG   CentralChannel;        //  40MHz central channel
} RT_802_11_LINK_STATUS, *PRT_802_11_LINK_STATUS;

typedef struct _RT_802_11_EVENT_LOG {
    LARGE_INTEGER   SystemTime;  // timestammp via NdisGetCurrentSystemTime()
    UCHAR           Addr[MAC_ADDR_LEN];
    USHORT          Event;       // EVENT_xxx
} RT_802_11_EVENT_LOG, *PRT_802_11_EVENT_LOG;

typedef struct _RT_802_11_EVENT_TABLE {
    ULONG       Num;
    ULONG       Rsv;     // to align Log[] at LARGE_INEGER boundary
    RT_802_11_EVENT_LOG   Log[MAX_NUM_OF_EVENT];
} RT_802_11_EVENT_TABLE, PRT_802_11_EVENT_TABLE;

typedef struct _RT_802_11_MAC_TABLE {
    ULONG       Num;
    RT_802_11_MAC_ENTRY Entry[MAX_LEN_OF_MAC_TABLE];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;

//
// Query/Set hardware types.
//
#define HARDWARE_MAC    0
#define HARDWARE_BBP    1
#define HARDWARE_RF     2
#define HARDWARE_EEP    3
#define HARDWARE_FUNC       5

// structure for query/set hardware register - MAC, BBP, RF register
typedef struct _RT_802_11_HARDWARE_REGISTER {
    ULONG   HardwareType;       // 0:MAC, 1:BBP, 2:RF register, 3:EEPROM
    ULONG   Offset;             // Q/S register offset addr
    ULONG   Data;               // R/W data buffer
} RT_802_11_HARDWARE_REGISTER, *PRT_802_11_HARDWARE_REGISTER;

// Access Unify CR
typedef struct _RT_802_11_CR_ACCESS {
    ULONG   HardwareType;       // 5 : Unify CR access
    ULONG   SetOrQuery;             // 0 : Query;  1 : Set
    ULONG   CR;                     // Q/S register offset addr
    ULONG   Data;               // R/W data buffer
} RT_802_11_CR_ACCESS, *PRT_802_11_CR_ACCESS;

// structure to tune BBP R17 "RX AGC VGC init"
//typedef struct _RT_802_11_RX_AGC_VGC_TUNING {
//    UCHAR   FalseCcaLowerThreshold;  // 0-255, def 10
//    UCHAR   FalseCcaUpperThreshold;  // 0-255, def 100
//    UCHAR   VgcDelta;                // R17 +-= VgcDelta whenever flase CCA over UpprThreshold
//                                     // or lower than LowerThresholdupper threshold
//    UCHAR   VgcUpperBound;           // max value of R17
//} RT_802_11_RX_AGC_VGC_TUNING, *PRT_802_11_RX_AGC_VGC_TUNING;

typedef struct _RT_802_11_AP_CONFIG {
    ULONG   EnableTxBurst;      // 0-disable, 1-enable
    ULONG   EnableTurboRate;    // 0-disable, 1-enable 72/100mbps turbo rate
    ULONG   IsolateInterStaTraffic;     // 0-disable, 1-enable isolation
    ULONG   HideSsid;           // 0-disable, 1-enable hiding
    ULONG   UseBGProtection;    // 0-AUTO, 1-always ON, 2-always OFF
    ULONG   UseShortSlotTime;   // 0-no use, 1-use 9-us short slot time
    USHORT  AgeoutTime;         // default 300, unit: sec
        USHORT  Rsv1;               // must be 0
    ULONG   SystemErrorBitmap;  // ignore upon SET, return system error upon QUERY
} RT_802_11_AP_CONFIG, *PRT_802_11_AP_CONFIG;

// structure to query/set STA_CONFIG
typedef struct _RT_802_11_STA_CONFIG {
    ULONG   bEnableTxBurst;      // 0-disable, 1-enable
    ULONG   EnableTurboRate;    // 0-disable, 1-enable 72/100mbps turbo rate
    ULONG   UseBGProtection;    // 0-AUTO, 1-always ON, 2-always OFF
    ULONG   UseShortSlotTime;   // 0-no use, 1-use 9-us short slot time when applicable
    ULONG   AdhocMode;          // 0-11b rates only (WIFI spec), 1 - b/g mixed, 2 - g only
    ULONG   HwRadioStatus;      // 0-OFF, 1-ON, default is 1, Read-Only
    ULONG   Rsv1;               // must be 0
    ULONG   SystemErrorBitmap;  // ignore upon SET, return system error upon QUERY
} RT_802_11_STA_CONFIG, *PRT_802_11_STA_CONFIG;

typedef struct _RT_802_11_ACL_ENTRY {
    UCHAR   Addr[MAC_ADDR_LEN];
    USHORT  Rsv;
} RT_802_11_ACL_ENTRY, *PRT_802_11_ACL_ENTRY;

typedef enum _ACL_POLICY {
    ACL_POLICY_DISABLE = 0,
    ACL_POLICY_POSITIVE_LIST = 1,
    ACL_POLICY_NEGATIVE_LIST = 2
} ACL_POLICY, *PACL_POLICY;

typedef struct _RT_802_11_ACL {
    ULONG   Policy;             // 0-disable, 1-positive list, 2-negative list
    ULONG   Num;
    RT_802_11_ACL_ENTRY Entry[MAX_LEN_OF_MAC_TABLE];
} RT_802_11_ACL, *PRT_802_11_ACL;

typedef struct _RT_802_11_WDS {
    ULONG                       Num;
    NDIS_802_11_MAC_ADDRESS     Entry[MAX_NUM_OF_WDS_LINK];
    ULONG                       KeyLength;
    UCHAR                       KeyMaterial[32];
} RT_802_11_WDS, *PRT_802_11_WDS;

typedef struct _RT_802_11_TX_RATES_ {
    UCHAR       SupRateLen; 
    UCHAR       SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR       ExtRateLen; 
    UCHAR       ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
} RT_802_11_TX_RATES, *PRT_802_11_TX_RATES;

// structure for DLS
typedef struct _RT_802_11_DLS_UI {
    USHORT                      TimeOut;        // unit: second , set by UI
    USHORT                      CountDownTimer; // unit: second , used by driver only
    NDIS_802_11_MAC_ADDRESS     MacAddr;        // set by UI
    UCHAR                       Status;         // 0: none , 1: wait STAkey, 2: finish DLS setup , set by driver only
    BOOLEAN                     Valid;          // 1: valid , 0: invalid , set by UI, use to setup or tear down DLS link
} RT_802_11_DLS_UI, *PRT_802_11_DLS_UI;

// structure for DLS
typedef struct _RT_802_11_DLS {
    USHORT                      TimeOut;        // Use to time out while slience, unit: second , set by UI
    USHORT                      CountDownTimer; // Use to time out while slience,unit: second , used by driver only
    NDIS_802_11_MAC_ADDRESS     MacAddr;        // set by UI
    UCHAR                       Status;         // 0: none , 1: wait STAkey, 2: finish DLS setup , set by driver only
    BOOLEAN                     Valid;          // 1: valid , 0: invalid , set by UI, use to setup or tear down DLS link
    MTK_TIMER_STRUCT         Timer;          // Use to time out while handshake
    UCHAR                       Wcid;
    PVOID                       pAd;
} RT_802_11_DLS, *PRT_802_11_DLS;

typedef enum _RT_802_11_DLS_MODE {
    DLS_NONE,
    DLS_WAIT_KEY,
    DLS_FINISH
} RT_802_11_DLS_MODE;

// Cipher suite type for mixed mode group cipher, P802.11i-2004
typedef enum _RT_802_11_CIPHER_SUITE_TYPE {
    Cipher_Type_NONE,
    Cipher_Type_WEP40,
    Cipher_Type_TKIP,
    Cipher_Type_RSVD,
    Cipher_Type_CCMP,
    Cipher_Type_WEP104
} RT_802_11_CIPHER_SUITE_TYPE, *PRT_802_11_CIPHER_SUITE_TYPE;


// Addon enum for WPA2
#define Ndis802_11AuthModeWPA2              ((NDIS_802_11_AUTHENTICATION_MODE) 6)
#define Ndis802_11AuthModeWPA2PSK           ((NDIS_802_11_AUTHENTICATION_MODE) 7)
#define Ndis802_11AuthModeWPA1WPA2          ((NDIS_802_11_AUTHENTICATION_MODE) 8)
#define Ndis802_11AuthModeWPA1PSKWPA2PSK    ((NDIS_802_11_AUTHENTICATION_MODE) 9)       // WPA1PSK/WPA2PSK mix mode
#define Ndis802_11AuthModeMax               ((NDIS_802_11_AUTHENTICATION_MODE) 10)      // Redefine to accomadate new enum value

#define Ndis802_11Encryption4Enabled        ((NDIS_802_11_WEP_STATUS)           8)      // TKIP/AES mix mode

// Add a new network type for P2P
#define Ralink802_11NetworkTypeP2P          ((NDIS_802_11_NETWORK_INFRASTRUCTURE) 7)    


// Definition of extra information code
#define GENERAL_LINK_UP         0x0         // Link is Up
#define GENERAL_LINK_DOWN       0x1         // Link is Down
#define HW_RADIO_OFF            0x2         // Hardware radio off
#define SW_RADIO_OFF            0x3         // Software radio off
#define AUTH_FAIL               0x4         // Open authentication fail
#define AUTH_FAIL_KEYS          0x5         // Shared authentication fail
#define ASSOC_FAIL              0x6         // Association failed
#define EAP_MIC_FAILURE         0x7         // Deauthencation because MIC failure
#define EAP_4WAY_TIMEOUT        0x8         // Deauthencation on 4-way handshake timeout
#define EAP_GROUP_KEY_TIMEOUT   0x9         // Deauthencation on group key handshake timeout
#define EAP_SUCCESS             0xa         // EAP succeed
#define DETECT_RADAR_SIGNAL     0xb         // Radar signal occur in current channel

#define EXTRA_INFO_CLEAR        0xffffffff


//
// ===========================================================================================================
//
#define MTK_OID_N6_QUERY_VENDOR                       (MTK_TOGGLE_OID_N6 | 0xFF0C0001)
#define MTK_OID_N6_QUERY_PHY_MODE               (MTK_TOGGLE_OID_N6 | 0xFF0C0002) 
#define MTK_OID_N6_SET_PHY_MODE                 (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_PHY_MODE)
#define MTK_OID_N6_QUERY_COUNTRY_REGION         (MTK_TOGGLE_OID_N6 | 0xFF0C0003)
#define MTK_OID_N6_SET_COUNTRY_REGION           (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_COUNTRY_REGION)
#define MTK_OID_N6_QUERY_EEPROM_VERSION         (MTK_TOGGLE_OID_N6 | 0xFF0C0004)
#define MTK_OID_N6_QUERY_FIRMWARE_VERSION       (MTK_TOGGLE_OID_N6 | 0xFF0C0005)
#define MTK_OID_N6_QUERY_CURRENT_CHANNEL_ID     (MTK_TOGGLE_OID_N6 | 0xFF0C0006)
#define MTK_OID_N6_QUERY_TX_POWER_LEVEL_1       (MTK_TOGGLE_OID_N6 | 0xFF0C0007)
#define MTK_OID_N6_SET_TX_POWER_LEVEL_1         (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_TX_POWER_LEVEL_1)
#define MTK_OID_N6_QUERY_HARDWARE_REGISTER      (MTK_TOGGLE_OID_N6 | 0xFF0C0008)
#define MTK_OID_N6_SET_HARDWARE_REGISTER        (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_HARDWARE_REGISTER)
#define MTK_OID_N6_QUERY_LINK_STATUS            (MTK_TOGGLE_OID_N6 | 0xFF0C0009)
#define MTK_OID_N6_QUERY_NOISE_LEVEL            (MTK_TOGGLE_OID_N6 | 0xFF0C000A)
#define MTK_OID_N6_QUERY_STATISTICS             (MTK_TOGGLE_OID_N6 | 0xFF0C000B)
#define MTK_OID_N6_SET_RESET_COUNTERS               (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF0C000C)
#define MTK_OID_N6_QUERY_EXTRA_INFO             (MTK_TOGGLE_OID_N6 | 0xFF0C000D)
#define MTK_OID_N6_QUERY_LAST_RX_RATE           (MTK_TOGGLE_OID_N6 | 0xFF0C000E)
#define MTK_OID_N6_QUERY_RSSI_0                 (MTK_TOGGLE_OID_N6 | 0xFF0C000F)
#define MTK_OID_N6_QUERY_RSSI_1                 (MTK_TOGGLE_OID_N6 | 0xFF0C0010)
#define MTK_OID_N6_QUERY_BALD_EAGLE_SSID    (MTK_TOGGLE_OID_N6 | 0x0D7101E1)
#define MTK_OID_N6_SET_BALD_EAGLE_SSID      (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_BALD_EAGLE_SSID)
//

//



// 11n
#define MTK_OID_N6_QUERY_LAST_TX_RATE           (MTK_TOGGLE_OID_N6 | 0xFF0C0021)
#define MTK_OID_N6_QUERY_RSSI_2                 (MTK_TOGGLE_OID_N6 | 0xFF0C0022)
#define MTK_OID_N6_QUERY_SNR_0                  (MTK_TOGGLE_OID_N6 | 0xFF0C0023)
#define MTK_OID_N6_QUERY_SNR_1                  (MTK_TOGGLE_OID_N6 | 0xFF0C0024)
#define MTK_OID_N6_QUERY_BATABLE                (MTK_TOGGLE_OID_N6 | 0xFF0C0025)
#define MTK_OID_N6_QUERY_IMME_BA_CAP            (MTK_TOGGLE_OID_N6 | 0xFF0C0026)
#define MTK_OID_N6_SET_IMME_BA_CAP              (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_IMME_BA_CAP)
#define MTK_OID_N6_SET_ADD_IMME_BA                  (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_BATABLE)
#define MTK_OID_N6_SET_TEAR_IMME_BA                 (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF0C0027)
#define MTK_OID_N6_QUERY_HT_PHYMODE             (MTK_TOGGLE_OID_N6 | 0xFF0C0028)
#define MTK_OID_N6_SET_HT_PHYMODE               (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_HT_PHYMODE)
#define MTK_OID_N6_QUERY_MAC_TABLE              (MTK_TOGGLE_OID_N6 | 0xFF0C0029)

#define MTK_OID_N6_QUERY_SNR_2                  (MTK_TOGGLE_OID_N6 | MTK_OID_N5_QUERY_SNR_2)
#define MTK_OID_N6_QUERY_STBC_RX_COUNT                (MTK_TOGGLE_OID_N6 | MTK_OID_N5_QUERY_STBC_RX_COUNT)
#define MTK_OID_N6_QUERY_STBC_TX_COUNT                (MTK_TOGGLE_OID_N6 | MTK_OID_N5_QUERY_STBC_TX_COUNT)
#define MTK_OID_N6_QUERY_SS_NUM             (MTK_TOGGLE_OID_N6 | MTK_OID_N5_QUERY_SS_NUM)
#define MTK_OID_N6_SET_SS_NUM                   (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N5_SET_SS_NUM)


//
// TODO: To be implement.
#define MTK_OID_N6_QUERY_DESIRED_RATES                (MTK_TOGGLE_OID_N6 | 0xFF0C0011)
#define MTK_OID_N6_QUERY_EVENT_MESSAGE          (MTK_TOGGLE_OID_N6 | 0xFF0C0018)
#define MTK_OID_N6_QUERY_EVENT_CAPABLE          (MTK_TOGGLE_OID_N6 | 0xFF0C0019)
#define MTK_OID_N6_QUERY_OP_MODE                (MTK_TOGGLE_OID_N6 | 0xFF0C001A)
#define MTK_OID_N6_SET_OP_MODE                  (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_OP_MODE)

#define MTK_OID_N6_QUERY_IEEE80211H             (MTK_TOGGLE_OID_N6 | 0xFF0C001C) 
#define MTK_OID_N6_SET_IEEE80211H               (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_IEEE80211H)
#define MTK_OID_N6_QUERY_AGGREGATION            (MTK_TOGGLE_OID_N6 | 0xFF0C001D) 
#define MTK_OID_N6_SET_AGGREGATION              (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_AGGREGATION)
#define MTK_OID_N6_QUERY_PRIVATE_MESSAGE        (MTK_TOGGLE_OID_N6 | 0xFF0C001F)
#define MTK_OID_N6_SET_PSPXLINK_MODE            (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF0C0020)

//
// WPS-related OIDs.
//
#define MTK_OID_N6_SET_WSC_MODE                                         (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF8C002A) // No query method for this OID.
#define MTK_OID_N6_SET_WSC_ASSOCIATE_IE                                 (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF8C002B)
#define MTK_OID_N6_SET_WSC_PROBEREQ_IE                                  (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF8C002C)
#define MTK_OID_N6_QUERY_WSC_CUSTOMIZED_IE_PARAM                        (MTK_TOGGLE_OID_N6 | 0xFF7101EA)
#define MTK_OID_N6_SET_WSC_CUSTOMIZED_IE_PARAM                          (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_WSC_CUSTOMIZED_IE_PARAM)
#define MTK_OID_N6_SET_WSC_PROFILE                                      (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF8C0049)
#define MTK_OID_N6_SET_WSC_ACTIVE_PROFILE                               (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF8C004A)
#define MTK_OID_N6_SET_WSC_CUSTOMIZE_DEVINFO                            (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF8C004B)
#define MTK_OID_N6_SET_WSC_PIN_CODE                                     (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF8C004D)
#define MTK_OID_N6_SET_WSC_HW_PBC_CUSTOMER_VENDOR_STATE_MACHINE  (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101A4)
#define MTK_OID_N5_SET_WSC_GETPROFILE_COMPLETE  (MTK_TOGGLE_OID_SET | 0x0D7101C2)   //Only use by Gemtek UI.
#define MTK_OID_N6_SET_WSC_EXTREG_MESSSAGE   (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101C3)

//
// Disable antenna diversity, set fixed antenna for RSSI test program
//
#define MTK_OID_N6_SET_DISABLE_ANTENNA_DIVERSITY          (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF7101EB)
#define MTK_OID_N6_SET_FIXED_MAIN_ANTENNA                 (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF7101EC)
#define MTK_OID_N6_SET_FIXED_AUX_ANTENNA                  (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF7101ED)

#define MTK_OID_N6_QUERY_WSC_STATUS                         (MTK_TOGGLE_OID_N6 | 0xFF0C002D)
#define MTK_OID_N6_QUERY_WSC_PROFILE                        (MTK_TOGGLE_OID_N6 | 0xFF0C002E)
#define MTK_OID_N6_QUERY_WSC_SSID                           (MTK_TOGGLE_OID_N6 | 0xFF7101E8)
#define MTK_OID_N6_QUERY_WSC_HW_PBC                         (MTK_TOGGLE_OID_N6 | 0xFF0C002F)
#define MTK_OID_N6_QUERY_EXTREG_EVENT_MESSAGE    (MTK_TOGGLE_OID_N6 | 0x0D7101C4)

#define MTK_OID_N6_QUERY_WSC_VERSION                        (MTK_TOGGLE_OID_N6 | 0x0D7101E2)
#define MTK_OID_N6_QUERY_WSC_EXTREG_PBC_ENRO_MAC            (MTK_TOGGLE_OID_N6 | 0x0D7101E3) 

#define MTK_OID_N6_SET_WSC_VERSION2                 (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_WSC_VERSION)
#define MTK_OID_N6_SET_USE_SEL_ENRO_MAC                         (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101E5)
#define MTK_OID_N6_SET_RECONFIG_AP                      (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101E6)

#define MTK_OID_N6_QUERY_WSC20_TESTBED               (MTK_TOGGLE_OID_N6 | 0x0D7101E7)
#define MTK_OID_N6_SET_WSC20_TESTBED                 (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101E7)

//
// Gemtek: During the WPS PBC walk time (120 seconds), the NIC MUST abort its connection attempt and signal a "session overlap" error 
// to the user if it discovers more than one Registrar in PBC mode.
//
#define MTK_OID_N6_SET_WSC_WPS_STATE_MACHINE_TERMINATION            (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101A0)

#define MTK_OID_N6_QUERY_INITIALIZATION_STATE  (MTK_TOGGLE_OID_N6 | 0x0D710181)

#define MTK_OID_N6_QUERY_SUPPRESS_SCAN          (MTK_TOGGLE_OID_N6 | 0x0D71013E)
#define MTK_OID_N6_SET_SUPPRESS_SCAN            (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET| 0x0D71013E)

#define MTK_WSC_OID_LIST  \
        MTK_OID_N6_SET_WSC_MODE,            \
        MTK_OID_N6_SET_WSC_ASSOCIATE_IE,        \
        MTK_OID_N6_SET_WSC_PROBEREQ_IE,     \
        MTK_OID_N6_QUERY_WSC_CUSTOMIZED_IE_PARAM    , \
        MTK_OID_N6_SET_WSC_CUSTOMIZED_IE_PARAM,     \
        MTK_OID_N6_SET_WSC_PROFILE,         \
        MTK_OID_N6_SET_WSC_ACTIVE_PROFILE,      \
        MTK_OID_N6_SET_WSC_CUSTOMIZE_DEVINFO,   \
        MTK_OID_N6_QUERY_WSC_STATUS,        \
        MTK_OID_N6_QUERY_WSC_PROFILE,       \
        MTK_OID_N6_QUERY_WSC_SSID,          \
        MTK_OID_N6_QUERY_WSC_HW_PBC,        \
        MTK_OID_N6_SET_PREFERRED_WPS_AP_PHY_TYPE,    \
        MTK_OID_N6_QUERY_PREFERRED_WPS_AP_PHY_TYPE,  \
        MTK_OID_N6_QUERY_WSC_AP_CHANNEL, \
        MTK_OID_N6_SET_WSC_WPS_STATE_MACHINE_TERMINATION,   \
        MTK_OID_N5_SET_WSC_GETPROFILE_COMPLETE,     \
        MTK_OID_N6_SET_WSC_VERSION2,            \
        MTK_OID_N6_SET_USE_SEL_ENRO_MAC,    \
        MTK_OID_N6_QUERY_WSC20_TESTBED, \
        MTK_OID_N6_SET_WSC20_TESTBED


#define MTK_OID_N6_SET_DESIRED_BSSID_LIST                       (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | OID_DOT11_DESIRED_BSSID_LIST)
#define MTK_OID_N6_SET_FLUSH_BSS_LIST                           (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | OID_DOT11_FLUSH_BSS_LIST)

#define MTK_OID_N6_QUERY_PREFERRED_WPS_AP_PHY_TYPE   (MTK_TOGGLE_OID_N6 | 0x0D710151)
#define MTK_OID_N6_SET_PREFERRED_WPS_AP_PHY_TYPE         (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_PREFERRED_WPS_AP_PHY_TYPE)

#define MTK_OID_N6_QUERY_WSC_AP_CHANNEL (MTK_TOGGLE_OID_N6 | 0x0D710152)

//WPS LED MODE 10 for Dlink WPS LED
#define MTK_OID_N6_SET_LED_WPS_MODE10                           (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0x0D7101A1)

// ACM / CAC
#define MTK_OID_N6_QUERY_WMM_ACM_CMD                        (MTK_TOGGLE_OID_N6 | 0x0D710153)
#define MTK_OID_N6_SET_WMM_ACM_CMD                      (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_WMM_ACM_CMD)

// CCX - reserve 0x0D710160 ~ 0x0D710180
#define MTK_OID_N5_QUERY_CCX_RX_AIRONET             0x0D710160
#define MTK_OID_N5_SET_CCX_TX_AIRONET               (MTK_TOGGLE_OID_SET | MTK_OID_N5_QUERY_CCX_RX_AIRONET)

#define MTK_OID_N5_SET_CCX_MFP                          0x0D710161
#define MTK_OID_N5_SET_CCX_DIAG_ACTION              0x0D710162
//
#define MTK_OID_N6_QUERY_EEPROM_BANDWIDTH       (MTK_TOGGLE_OID_N6 | 0x0D710165)

//
// SoftAP
//
#define MTK_OID_N6_QUERY_AUTHENTICATION_MODE    (MTK_TOGGLE_OID_N6 | 0xFF0C0035)
#define MTK_OID_N6_SET_AUTHENTICATION_MODE      (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_AUTHENTICATION_MODE)
#define MTK_OID_N6_QUERY_WEP_STATUS             (MTK_TOGGLE_OID_N6 | 0xFF0C0036)
#define MTK_OID_N6_SET_WEP_STATUS               (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_WEP_STATUS)
// Renamed to reflect better the extended set of encryption status
#define MTK_OID_N6_QUERY_ENCRYPTION_STATUS      (MTK_TOGGLE_OID_N6 | MTK_OID_N6_QUERY_WEP_STATUS)
#define MTK_OID_N6_SET_ENCRYPTION_STATUS        (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_WEP_STATUS)
#define MTK_OID_N6_SET_ADD_KEY                  (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF0C0038)
#define MTK_OID_N6_SET_REMOVE_KEY               (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF0C0039)
#define MTK_OID_N6_QUERY_RADIUS_DATA            (MTK_TOGGLE_OID_N6 | 0xFF0C003A)
#define MTK_OID_N6_SET_RADIUS_DATA              (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_RADIUS_DATA)
#define MTK_OID_N6_QUERY_CONFIGURATION          (MTK_TOGGLE_OID_N6 | 0xFF0C003B)
#define MTK_OID_N6_SET_CONFIGURATION            (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_CONFIGURATION)




#define MTK_OID_N6_QUERY_RX_AGC_VGC_TUNING      (MTK_TOGGLE_OID_N6 | 0xFF0C003E)
#define MTK_OID_N6_SET_RX_AGC_VGC_TUNING        (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_RX_AGC_VGC_TUNING)
#define MTK_OID_N6_QUERY_AP_CONFIG              (MTK_TOGGLE_OID_N6 | 0xFF0C0040)
#define MTK_OID_N6_SET_AP_CONFIG                (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_AP_CONFIG)
#define MTK_OID_N6_QUERY_WPA_REKEY              (MTK_TOGGLE_OID_N6 | 0xFF0C0041)
#define MTK_OID_N6_SET_WPA_REKEY                (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_WPA_REKEY)
#define MTK_OID_N6_QUERY_EVENT_TABLE            (MTK_TOGGLE_OID_N6 | 0xFF0C0043)
#define MTK_OID_N6_SET_DESIRED_RATES            (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF0C0045)
#define MTK_OID_N6_SET_ACL                      (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF0C0046)
#define MTK_OID_N6_SET_MIC_ERROR            (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF0C0047)
#define MTK_OID_N6_SET_WDS                      (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF0C0048)
#define MTK_OID_N6_SET_USB_CYCLE_PORT           (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | 0xFF0C0060)
#define MTK_OID_N6_SET_DISCONNECT_REQUEST           (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | OID_DOT11_DISCONNECT_REQUEST)
#define MTK_OID_N6_QUERY_USB_VERSION              (MTK_TOGGLE_OID_N6 | 0xFF710155)

#define MTK_OID_N6_QUERY_HW_ARCHITECTURE_VERSION            (MTK_TOGGLE_OID_N6 | 0x0D7C0050)

//
// VHT (802.11ac)
//
#define MTK_OID_N6_QUERY_HW_CAPABILITIES        (MTK_TOGGLE_OID_N6 | 0x0D710335)  // Report HW feature in current adapter. ex. VHT/ HwWAPI...
#define MTK_OID_N6_QUERY_FIXED_RATE             (MTK_TOGGLE_OID_N6 | 0x0D710336)  // UI use this OID to get current rate setting
#define MTK_OID_N6_SET_FIXED_RATE               (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_FIXED_RATE)

//
// UNIFY MAC
//
#define MTK_OID_N6_QUERY_WTBL_INFO                (MTK_TOGGLE_OID_N6 | 0xFF710501)
#define MTK_OID_N6_QUERY_TEST_TX_NULL_FRAME       (MTK_TOGGLE_OID_N6 | 0xFF710502)
#define MTK_OID_N6_SET_TEST_TX_NULL_FRAME       (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_TEST_TX_NULL_FRAME)
#define MTK_OID_N6_QUERY_RADIO_STATUS             (MTK_TOGGLE_OID_N6 | 0xFF710503)
#define MTK_OID_N6_SET_RADIO                    (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_RADIO_STATUS)
#define MTK_OID_N6_QUERY_MIB                      (MTK_TOGGLE_OID_N6 | 0xFF710504)
#define MTK_OID_N6_QUERY_POWER_MANAGEMENT         (MTK_TOGGLE_OID_N6 | 0xFF710505)
#define MTK_OID_N6_SET_POWER_MANAGEMENT         (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_POWER_MANAGEMENT)
#define MTK_OID_N6_QUERY_RF_TEST                  (MTK_TOGGLE_OID_N6 | 0xFF710506)
#define MTK_OID_N6_SET_RF_TEST                  (MTK_TOGGLE_OID_N6 | MTK_TOGGLE_OID_SET | MTK_OID_N6_QUERY_RF_TEST)


//CCX OIDS
#define MTK_OID_N6_SET_CCX_ASSOC_INFO                  (MTK_TOGGLE_OID_N6 | 0xFF000003)
#define MTK_OID_N6_SET_CCX_RM_REQUEST                  (MTK_TOGGLE_OID_N6 | 0xFF000004)
#define MTK_OID_N6_SET_CCX_NEIGHBOR_LIST               (MTK_TOGGLE_OID_N6 | 0xFF000005)
#define MTK_OID_N6_SET_CCX_ROAM                        (MTK_TOGGLE_OID_N6 | 0xFF000006)
#define MTK_OID_N6_SET_CCX_MFP_STATISTICS              (MTK_TOGGLE_OID_N6 | 0xFF000007)
#define MTK_OID_N6_SET_CCX_KEEP_ALIVE_REFRESH          (MTK_TOGGLE_OID_N6 | 0xFF000008)
#define MTK_OID_N6_SET_CCX_STA_STATISTICS_2            (MTK_TOGGLE_OID_N6 | 0xFF00000A)
#define MTK_OID_N6_SET_CCX_FW_VERSION                  (MTK_TOGGLE_OID_N6 | 0xFF00000B)
#define MTK_OID_N6_SET_CCX_SERVICE_CAPABILITY          (MTK_TOGGLE_OID_N6 | 0xFF00000C)
#define MTK_OID_N6_SET_CCX_ANTENNA_DATA                (MTK_TOGGLE_OID_N6 | 0xFF00000D)
#define MTK_OID_N6_SET_CCX_MANUFACTURER_SERIAL_NUM     (MTK_TOGGLE_OID_N6 | 0xFF00000F)
#define MTK_OID_N6_SET_CCX_MANUFACTURER_MODEL          (MTK_TOGGLE_OID_N6 | 0xFF000010)
#define MTK_OID_N6_SET_CCX_MANUFACTURER_ID             (MTK_TOGGLE_OID_N6 | 0xFF000011)
#define MTK_OID_N6_SET_CCX_MANUFACTURER_OUI            (MTK_TOGGLE_OID_N6 | 0xFF000012)
#define MTK_OID_N6_SET_CCX_ENTERPRISE_PHONE_NUM        (MTK_TOGGLE_OID_N6 | 0xFF000013)
#define MTK_OID_N6_SET_CCX_CELL_PHONE_NUM              (MTK_TOGGLE_OID_N6 | 0xFF000014)
#define MTK_OID_N6_SET_CCX_RADIO_CHANNELS              (MTK_TOGGLE_OID_N6 | 0xFF000015)
#define MTK_OID_N6_SET_CCX_DRIVER_VERSION              (MTK_TOGGLE_OID_N6 | 0xFF000016)
#define MTK_OID_N6_SET_CCX_LAST_BCN_TIME               (MTK_TOGGLE_OID_N6 | 0xFF000017)
#define MTK_OID_N6_SET_CCX_FRAME_LOGGING_MODE          (MTK_TOGGLE_OID_N6 | 0xFF000018)
#define MTK_OID_N6_SET_CCX_TSF                         (MTK_TOGGLE_OID_N6 | 0xFF000019)
#define MTK_OID_N6_SET_CCX_DIAGNOSTICS_MODE            (MTK_TOGGLE_OID_N6 | 0xFF00001A)
#define MTK_OID_N6_SET_CCX_NUM_TX_BUFFER               (MTK_TOGGLE_OID_N6 | 0xFF00001B)

#define CCX_ERROR_SUCCESS               0x00000000
#define CCX_ERROR_UNKNOWN_VERSION      (0x80000000 | 1305)          //ERROR_UNKNOWN_REVISION
#define CCX_ERROR_FAILED               (0x80000000 | 1627)          //ERROR_FUNCTION_FAILED
#define CCX_ERROR_INTERNAL             (0x80000000 | 1359)          //ERROR_INTERNAL_ERROR
#define CCX_ERROR_INVALID_PARAM        (0x80000000 | 87)            //ERROR_INVALID_PARAMETER
#define CCX_ERROR_INVALID_LENGTH       (0x80000000 | 24)            //ERROR_BAD_LENGTH
#define CCX_ERROR_INVALID_DATA         (0x80000000 | 13)            //ERROR_INVALID_DATA
#define CCX_ERROR_NOT_SUPPORTED        (0x80000000 | 50)            //ERROR_NOT_SUPPORTED
#define CCX_ERROR_BUFFER_TOO_SHORT     (0x80000000 | 122)           //ERROR_INSUFFICIENT_BUFFER
#define CCX_ERROR_TIMEOUT              (0x80000000 | 258)           //WAIT_TIMEOUT

#define MTK_CCX_OID_LIST             \
    MTK_OID_N6_SET_CCX_ASSOC_INFO,                  \
    MTK_OID_N6_SET_CCX_RM_REQUEST,                  \
    MTK_OID_N6_SET_CCX_NEIGHBOR_LIST,               \
    MTK_OID_N6_SET_CCX_ROAM,                        \
    MTK_OID_N6_SET_CCX_MFP_STATISTICS,              \
    MTK_OID_N6_SET_CCX_KEEP_ALIVE_REFRESH,          \
    MTK_OID_N6_SET_CCX_STA_STATISTICS_2,            \
    MTK_OID_N6_SET_CCX_FW_VERSION,                  \
    MTK_OID_N6_SET_CCX_SERVICE_CAPABILITY,          \
    MTK_OID_N6_SET_CCX_ANTENNA_DATA,                \
    MTK_OID_N6_SET_CCX_MANUFACTURER_SERIAL_NUM,     \
    MTK_OID_N6_SET_CCX_MANUFACTURER_MODEL,          \
    MTK_OID_N6_SET_CCX_MANUFACTURER_ID,             \
    MTK_OID_N6_SET_CCX_MANUFACTURER_OUI,            \
    MTK_OID_N6_SET_CCX_ENTERPRISE_PHONE_NUM,        \
    MTK_OID_N6_SET_CCX_CELL_PHONE_NUM,              \
    MTK_OID_N6_SET_CCX_RADIO_CHANNELS,              \
    MTK_OID_N6_SET_CCX_DRIVER_VERSION,              \
    MTK_OID_N6_SET_CCX_LAST_BCN_TIME,               \
    MTK_OID_N6_SET_CCX_FRAME_LOGGING_MODE,          \
    MTK_OID_N6_SET_CCX_TSF,                    \
    MTK_OID_N6_SET_CCX_DIAGNOSTICS_MODE,            \
    MTK_OID_N6_SET_CCX_NUM_TX_BUFFER           
// ===================================================================================================

#define MTK_OID_LIST         \
    MTK_OID_N6_QUERY_VENDOR,                      \
    MTK_OID_N6_QUERY_PHY_MODE,              \
    MTK_OID_N6_SET_PHY_MODE,                \
    MTK_OID_N6_QUERY_COUNTRY_REGION,        \
    MTK_OID_N6_SET_COUNTRY_REGION,          \
    MTK_OID_N6_QUERY_EEPROM_VERSION,        \
    MTK_OID_N6_QUERY_FIRMWARE_VERSION,      \
    MTK_OID_N6_QUERY_CURRENT_CHANNEL_ID,    \
    MTK_OID_N6_QUERY_TX_POWER_LEVEL_1,      \
    MTK_OID_N6_SET_TX_POWER_LEVEL_1,        \
    MTK_OID_N6_QUERY_HARDWARE_REGISTER,     \
    MTK_OID_N6_SET_HARDWARE_REGISTER,       \
    MTK_OID_N6_QUERY_LINK_STATUS,           \
    MTK_OID_N6_QUERY_NOISE_LEVEL,           \
    MTK_OID_N6_QUERY_STATISTICS,            \
    MTK_OID_N6_SET_RESET_COUNTERS,              \
    MTK_OID_N6_QUERY_EXTRA_INFO,            \
    MTK_OID_N6_QUERY_LAST_RX_RATE,          \
    MTK_OID_N6_QUERY_RSSI_0,                \
    MTK_OID_N6_QUERY_RSSI_1,                \
    MTK_OID_N6_QUERY_LAST_TX_RATE,          \
    MTK_OID_N6_QUERY_RSSI_2,                \
    MTK_OID_N6_QUERY_SNR_0,                 \
    MTK_OID_N6_QUERY_SNR_1,             \
    MTK_OID_N6_QUERY_CONFIGURATION,         \
    MTK_OID_N6_SET_CONFIGURATION,           \
    MTK_OID_N6_QUERY_AUTHENTICATION_MODE,   \
    MTK_OID_N6_SET_AUTHENTICATION_MODE,     \
    MTK_OID_N6_QUERY_ENCRYPTION_STATUS,     \
    MTK_OID_N6_SET_ENCRYPTION_STATUS,       \
    MTK_OID_N6_SET_REMOVE_KEY,              \
    MTK_OID_N6_SET_ADD_KEY,                 \
    MTK_OID_N6_QUERY_EVENT_TABLE,           \
    MTK_OID_N6_QUERY_MAC_TABLE,             \
    MTK_OID_N6_SET_ACL,                     \
    MTK_OID_N6_QUERY_OP_MODE,               \
    MTK_OID_N6_SET_OP_MODE,                 \
    MTK_OID_N6_QUERY_AP_CONFIG,             \
    MTK_OID_N6_SET_AP_CONFIG,               \
    MTK_OID_N6_QUERY_BATABLE,               \
    MTK_OID_N6_QUERY_IMME_BA_CAP,           \
    MTK_OID_N6_SET_IMME_BA_CAP,             \
    MTK_OID_N6_SET_ADD_IMME_BA,                 \
    MTK_OID_N6_SET_TEAR_IMME_BA,                \
    MTK_OID_N6_QUERY_HT_PHYMODE,            \
    MTK_OID_N6_SET_HT_PHYMODE,              \
    MTK_OID_N6_SET_DESIRED_BSSID_LIST,          \
    MTK_OID_N6_SET_DISCONNECT_REQUEST,          \
    MTK_OID_N6_QUERY_WPA_REKEY,             \
    MTK_OID_N6_SET_WPA_REKEY,               \
    MTK_OID_N6_SET_FLUSH_BSS_LIST,              \
                                        \
    MTK_OID_N6_QUERY_STA_CONFIG,     \
    MTK_OID_N6_SET_STA_CONFIG,       \
    MTK_OID_N6_SET_CCX20_INFO,           \
    MTK_OID_N6_QUERY_PREAMBLE,       \
    MTK_OID_N6_SET_PREAMBLE,         \
                                        \
    MTK_OID_N6_QUERY_APSD_SETTING,   \
    MTK_OID_N6_SET_APSD_SETTING,     \
    MTK_OID_N6_QUERY_APSD_PSM,       \
    MTK_OID_N6_SET_APSD_PSM,         \
    MTK_OID_N6_QUERY_WMM,            \
    MTK_OID_N6_SET_WMM,              \
    MTK_OID_N6_QUERY_DLS,            \
    MTK_OID_N6_SET_DLS,              \
    MTK_OID_N6_QUERY_DLS_PARAM,      \
    MTK_OID_N6_SET_DLS_PARAM,        \
                                        \
    MTK_OID_N6_SET_CIPHER_KEY_MAPPING_KEY,\
    MTK_OID_N6_SET_CIPHER_DEFAULT_KEY,    \
    MTK_OID_N6_QUERY_MFP_CONTROL,    \
    MTK_OID_N6_SET_MFP_CONTROL,      \
    MTK_OID_N6_QUERY_PORT_USAGE,    \
    MTK_OID_N6_SET_FRAGMENTATION_THRESHOLD,   \
    MTK_OID_N6_SET_RTS_THRESHOLD,         \
    MTK_OID_N6_SET_POWER_MGMT_REQUEST,    \
    MTK_OID_N6_QUERY_USB_VERSION,         \
    MTK_OID_N6_QUERY_INITIALIZATION_STATE,  \
    MTK_WSC_OID_LIST,                         \
    MTK_OID_N6_SET_LED_WPS_MODE10,              \
    MTK_OID_N6_SET_WSC_HW_PBC_CUSTOMER_VENDOR_STATE_MACHINE, \
    MTK_OID_N6_SET_WSC_PIN_CODE,    \
                                        \
    MTK_OID_N6_SET_USB_CYCLE_PORT,      \
    MTK_OID_N6_QUERY_MANUAL_CH_LIST, \
    MTK_OID_N5_QUERY_BT_HCI_GET_CMD,       \
    MTK_OID_N5_QUERY_BT_HCI_GET_ACL_DATA,  \
    MTK_OID_N5_SET_BT_HCI_SEND_CMD,  \
    MTK_OID_N5_SET_BT_HCI_SEND_ACL_DATA, \
    MTK_OID_N6_QUERY_EEPROM_BANDWIDTH,\
    MTK_OID_N6_SET_P2P_CONFIG,  \
    MTK_OID_N6_SET_P2P_PERST_TAB,   \
    MTK_OID_N6_QUERY_P2P_STATUS,    \
    MTK_OID_N6_QUERY_WIFI_DIRECT_LINKSTATE,     \
    MTK_OID_N6_QUERY_P2P_MAC_INFO,  \
    MTK_OID_N6_SET_P2P_EVENT,   \
    MTK_OID_N6_SET_P2P_DISCONNECT_PER_CONNECTION,   \
    MTK_OID_N6_SET_P2P_EVENT_READY, \
    MTK_OID_N6_SET_P2P_IP,  \
    MTK_OID_N6_SET_P2P_GO_PASSPHRASE,   \
    MTK_OID_N6_QUERY_P2P_PERSIST,  \
    MTK_OID_N6_SET_P2P_PERSIST,             \
    MTK_OID_N6_QUERY_SIGMA_P2P_STATUS,      \
    MTK_OID_N6_QUERY_SIGMA_WPS_PINCODE, \
    MTK_OID_N6_QUERY_SIGMA_PSK,         \
    MTK_OID_N6_QUERY_BALD_EAGLE_SSID,\
    MTK_OID_N6_SET_BALD_EAGLE_SSID, \
    MTK_OID_N6_QUERY_EXTREG_EVENT_MESSAGE,   \
    MTK_OID_N6_SET_WSC_EXTREG_MESSSAGE, \
    MTK_OID_N6_SET_SUPPRESS_SCAN, \
    MTK_OID_N6_QUERY_SUPPRESS_SCAN, \
    MTK_OID_N6_SET_CUSTOMER_BT_RADIO_STATUS, \
    MTK_OID_N6_QUERY_SNR_2, \
    MTK_OID_N6_QUERY_STBC_RX_COUNT, \
    MTK_OID_N6_QUERY_STBC_TX_COUNT, \
    MTK_OID_N6_QUERY_SS_NUM, \
    MTK_OID_N6_SET_SS_NUM,  \
                        \
    MTK_OID_N5_QUERY_STBC_RX_COUNT,    \
    MTK_OID_N5_QUERY_STBC_TX_COUNT,        \
    MTK_OID_N5_QUERY_SS_NUM, \
    MTK_OID_N5_SET_SS_NUM,   \
    MTK_OID_N5_QUERY_SNR_2,    \
                                \
    MTK_OID_N5_SET_TXBF_ETX_BF_TIMEOUT, \
    MTK_OID_N5_SET_TXBF_ETX_BF_EN,  \
    MTK_OID_N5_SET_TXBF_ETX_BF_NON_COMPRESSED_FEEDBACK, \
    MTK_OID_N6_QUERY_WSC_VERSION,   \
    MTK_OID_N6_QUERY_WSC_EXTREG_PBC_ENRO_MAC,   \
    MTK_OID_N6_SET_RECONFIG_AP, \
    MTK_OID_N6_SET_DISABLE_ANTENNA_DIVERSITY,   \
    MTK_OID_N6_SET_FIXED_MAIN_ANTENNA,  \
    MTK_OID_N6_SET_FIXED_AUX_ANTENNA, \
    MTK_OID_N6_QUERY_HW_ARCHITECTURE_VERSION,           \
    MTK_OID_N6_QUERY_HW_CAPABILITIES,                   \
    MTK_OID_N6_SET_FIXED_RATE,  \
    MTK_OID_N6_QUERY_WTBL_INFO,   \
    MTK_OID_N6_QUERY_TEST_TX_NULL_FRAME,  \
    MTK_OID_N6_SET_TEST_TX_NULL_FRAME,  \
    MTK_OID_N6_QUERY_RADIO_STATUS,    \
    MTK_OID_N6_SET_RADIO,   \
    MTK_OID_N6_QUERY_MIB,     \
    MTK_OID_N6_QUERY_POWER_MANAGEMENT,    \
    MTK_OID_N6_SET_POWER_MANAGEMENT,    \
    MTK_OID_N6_QUERY_RF_TEST,   \
    MTK_OID_N6_SET_RF_TEST


BOOLEAN
FORCEINLINE
GetRequiredListSize(
    __in ULONG ElemOffset,
    __in ULONG ElemSize,
    __in ULONG ElemCount,
    __out PULONG RequiredSize
    )
{
    ElemCount = (ElemCount == 0) ? 1 : ElemCount;
    
    if (RtlULongMult(ElemSize, ElemCount, RequiredSize) != STATUS_SUCCESS)
    {
        return FALSE;
    }

    if (RtlULongAdd(*RequiredSize, ElemOffset, RequiredSize) != STATUS_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
}

// For BTHS

#define IOCTL_HCI_WRITE \
    CTL_CODE (FILE_DEVICE_NETWORK, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HCI_HALT_NOTIFY \
    CTL_CODE (FILE_DEVICE_NETWORK, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
// the following is used when METHOD_IN_DIRECT
#define IOCTL_HCI_WRITE \
    CTL_CODE (FILE_DEVICE_NETWORK, 0x800, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
*/

#define IOCTL_HCI_READ \
    CTL_CODE (FILE_DEVICE_NETWORK, 0x800, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

// may have non-4bytes aligment
#pragma pack( push, struct_pack1 )
#pragma pack(1)
//BTHS structure
#define BTHS_MAX_KEY_LEN 32
#define MAX_EVENT_DATA_LEN      255
#define MAX_IRP_BUFF_SIZE           32
#define MAX_EVENT_IRP_BUFF_SIZE 32
#define MAX_LLH_CNT             16
#define MAX_CONDITION_SIZE      8
//#define   EVENT_BUFFER_QUEUE_CNT  8
#define BLACKOUT_BUFFER_SIZE        32
#define BTHS_TOTAL_BANDWIDTH    0x6000 // 24Mbps
#define BTHS_MAX_GUARANTEED_BANDWIDTH   0x6000 // 24Mbps
#define Max80211PALPDUSize          1400    
#define MAX80211AMPASSOCLEN         672
#define MinGUserPrio                    4
#define MaxGuserPrio                    7
#define BEUserPrio0                     0
#define BEUserPrio1                     3
#define Max80211BeaconPeriod            2000    //2000 msec
#define MAX_FLUSH_TIMEOUT           1000    //1000 microseconds
#define BEST_EFFORT_FLUSH_TIMEOUT   1000    //1000 microseconds, this value should not be greater than MAX FLUSH TIMEOUT

typedef struct _EXTENDED_FLOW_SPEC
{
    UCHAR       Identifier;
    UCHAR       Service_Type;
    USHORT      Max_SDU_Size;
    ULONG       SDU_Inter_arr_time;
    ULONG       Access_Latency;
    ULONG       Flush_Timeout;
} EXTENDED_FLOW_SPEC, *PEXTENDED_FLOW_SPEC;

#pragma pack(pop, struct_pack1)
#endif
