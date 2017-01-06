/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2005, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rt2870.h

    Abstract:
    RT2860usb ASIC  related definition & structures

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Jan Lee 02-13-2006    created

*/

#ifndef __EEPROM_H__
#define __EEPROM_H__
//-------------------------------------------------------------------------
// EEPROM definition
//-------------------------------------------------------------------------
#define EEDO                        0x08
#define EEDI                        0x04
#define EECS                        0x02
#define EESK                        0x01
#define EERL                        0x80

#define EEPROM_WRITE_OPCODE     0x05
#define EEPROM_READ_OPCODE      0x06
#define EEPROM_EWDS_OPCODE      0x10
#define EEPROM_EWEN_OPCODE      0x13

#define NUM_EEPROM_BBP_PARMS        19
#define NUM_EEPROM_TX_G_PARMS           7

#define EEPROM_NIC_CFG1_OFFSET              0
#define EEPROM_NIC_CFG2_OFFSET              1
#define EEPROM_NIC_COUNTRY_REGION_OFFSET    2
#define EEPROM_BBP_ARRAY_OFFSET         3

// For manual country region
#define EEPROM_BG_BAND_MANUAL_CH    0
#define EEPROM_A_BAND_MANUAL_CH_0   1
#define EEPROM_A_BAND_MANUAL_CH_1   2

#define EEPROM_MAC_ADDRESS          0x04

#define EEPROM_BG_BAND_MANUAL_CH_OFFSET 0x118
#define EEPROM_A_BAND_MANUAL_CH_0_OFFSET    0x11A
#define EEPROM_A_BAND_MANUAL_CH_1_OFFSET    0x11C
#define MAX_NUM_OF_MANUAL_REGION    3       // 2.4GHz: 0x100~0x101, 5GHz: 0X102~0X105

#define EEPROM_NIC1_OFFSET      0x34        // The address is from NIC config 0, not BBP register ID
#define EEPROM_NIC2_OFFSET      0x36        // The address is from NIC config 0, not BBP register ID
#define EEPROM_COUNTRY_REGION_CODE_FOR_5G_BAND  0x38
#define EEPROM_NIC3_OFFSET          0x42    // Nic configuration register 2
#define EEPROM_LED_MODE_SETTING     0x3B
#define EEPROM_LED1_OFFSET          0x3c
#define EEPROM_LED2_OFFSET          0x3e
#define EEPROM_LED3_OFFSET          0x40
#define EEPROM_LNA_OFFSET           0x44

#define EEPROM_VERSION_OFFSET       0x02
#define EEPROM_FREQ_OFFSET      0x3a
#define EEPROM_RSSI_BG_OFFSET       0x46
#define EEPROM_TXMIXER_GAIN_2_4G    0x48
#define EEPROM_RSSI_A_OFFSET        0x4a
#define EEPROM_TXMIXER_GAIN_5G  0x4c
#define EEPROM_DEFINE_MAX_TXPWR     0x4e
#define EEPROM_TXPOWER_BYRATE_20MHZ_2_4G    0xde    // 20MHZ 2.4G tx power.
#define EEPROM_TXPOWER_BYRATE_40MHZ_2_4G    0xee    // 40MHZ 2.4G tx power.
#define EEPROM_TXPOWER_BYRATE_20MHZ_5G      0xfa    // 20MHZ 5G tx power.
#define EEPROM_TXPOWER_BYRATE_40MHZ_5G      0x10a   // 40MHZ 5G tx power.
#define EEPROM_TXPOWER_DELTA        0x50    // 20MHZ AND 40 MHZ use different power. This is delta in 40MHZ.
#define EEPROM_TXPOWER_DELTA_BW80   0xD2
#define VALID_EEPROM_VERSION        1
#define EEPROM_LNA_OFFSET           0x44

// PairKeyMode definition
#define PKMODE_NONE                 0
#define PKMODE_WEP64                1
#define PKMODE_WEP128               2
#define PKMODE_TKIP                 3
#define PKMODE_AES                  4
#define PKMODE_CKIP64               5
#define PKMODE_CKIP128              6
#define PKMODE_TKIP_NO_MIC          7       // MIC appended by driver: not a valid value in hardware key table

//
// Extended EEPROM format (EEPROM_EXT_XXX)
//

//
// NIC configuration #2
//
#define EEPROM_EXT_NIC_CONFIGURATION_2                                  0x38

//
// Country region code for 5G band
//
#define EEPROM_EXT_COUNTRY_REGION_CODE_FOR_5G_BAND                  0x3F

//
// Maximum Tx power for 2.4 GHz and 5 GHz band
//
#define EEPROM_EXT_MAX_TX_POWER_OVER_2DOT4G_AND_5G                  0x40

//
// Frequency offset
//
#define EEPROM_EXT_FREQUENCY_OFFSET                                 0x44

//
// LED mode setting
//
#define EEPROM_EXT_LED_MODE_SETTING                                 0x45

//
// LED A/G configuration
//
#define EEPROM_EXT_LED_AG_CONFIGURATION                             0x46

//
// LED ACT configuration
//
#define EEPROM_EXT_LED_ACT_CONFIGURATION                                0x48

//
// LED A/G/ACT polarity
//
#define EEPROM_EXT_LED_AG_ACT_POLARITY                                  0x4A

//
// External LNA gain for 2.4 GHz band
//
#define EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_2DOT4G                     0x4C

//
// External LNA gain for 5 GHz band (channel #36~#64)
//
#define EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_5G_OVER_CH36_TO_CH64       0x4D

//
// External LNA gain for 5 GHz band (channel #100~#128)
//
#define EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_5G_OVER_CH100_TO_CH128     0x4E

//
// External LNA gain for 5 GHz band (channel #132~#165)
//
#define EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_5G_OVER_CH132_TO_CH165     0x4F

//
// RSSI0 offset for 2.4 GHz band
//
#define EEPROM_EXT_RSSI0_OVER_2DOT4G                                    0x50

//
// RSSI1 offset for 2.4 GHz band
//
#define EEPROM_EXT_RSSI1_OVER_2DOT4G                                    0x51

//
// RSSI2 offset for 2.4 GHz band
//
#define EEPROM_EXT_RSSI2_OVER_2DOT4G                                    0x52

//
// RSSI0 offset for 5 GHz band
//
#define EEPROM_EXT_RSSI0_OVER_5G                                        0x54

//
// RSSI1 offset for 5 GHz band
//
#define EEPROM_EXT_RSSI1_OVER_5G                                        0x55

//
// RSSI2 offset for 5 GHz band
//
#define EEPROM_EXT_RSSI2_OVER_5G                                        0x56

//
// Tx0 power over 2.4 GHz
//
#define EEPROM_EXT_TX0_OVER_2DOT4G                                      0x60

//
// Tx1 power over 2.4 GHz
//
#define EEPROM_EXT_TX1_OVER_2DOT4G                                      0x6E

//
// Tx2 power over 2.4 GHz
//
#define EEPROM_EXT_TX2_OVER_2DOT4G                                      0x7C

//
// Tx0 power over 5 GHz
//
#define EEPROM_EXT_TX0_OVER_5G                                          0x96

//
// Tx1 power over 5 GHz
//
#define EEPROM_EXT_TX1_OVER_5G                                          0xCA

//
// Tx2 power over 5 GHz
//
#define EEPROM_EXT_TX2_OVER_5G                                          0xFE

//
// Tx power delta TSSI bounday over 2.4 GHz
//
#define EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G                  0x8A

//
// Tx power delta TSSI bounday over 5 GHz
//
#define EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G                      0x134

//
// Tx ALC step value for 2.4 GHz
//
#define EEPROM_EXT_TX_ALC_STEP_VALUE_OVER_2DOT4G                        0x93

//
// Tx ALC step value for 5 GHz
//
#define EEPROM_EXT_TX_ALC_STEP_VALUE_OVER_5G                            0x13D

//
// Tx power control over BW20 at 2.4G
//
#define EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G                     0x140

//
// Tx power control over BW40 at 2.4G
//
#define EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G                     0x150

//
// Tx power control over BW20 at 5G
//
#define EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G                         0x160

//
// Tx power control over BW40 at 5G
//
#define EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G                         0x170

//
// The 2.4G manual channel
//
#define EEPROM_EXT_2DOT4G_MANUAL_CHANNEL_OFFSET                     0x190

//
// The 5G manual channel (part #1)
//
#define EEPROM_EXT_5G_MANUAL_CAHNNEL_PART_ONE_OFFSET                    0x192

//
// The 5G manual channel (part #2)
//
#define EEPROM_EXT_5G_MANUAL_CHANNEL_PART_TWO_OFFSET                0x194


//
// Delta Power For A/G Band (PART #1)
//
#define EEPROM_DELTA_POWER_FOR_BAND_PART_ONE_OFFSET             0x1C0

//
// Delta Power For A/G Band (PART #2)
//
#define EEPROM_DELTA_POWER_FOR_BAND_PART_TWO_OFFSET             0x1C2

#endif  // __EEPROM_H__